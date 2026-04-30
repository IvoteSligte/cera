#include "parser.h"
#include "alloc.h"
#include "ast.h"
#include "ast_macro.h"
#include "offset.h"

USE_AST_MACRO_HEADER; // silence unused ast_macro.h warning

#include <stdarg.h>

typedef ListAllocator Allocator;
typedef ASTNode Node;
typedef ASTNodeArray NodeArray;

Node *yield(Allocator *allocator, Node node) {
  Node *dest = la_calloc(allocator, sizeof(Node));
  *dest = node;
  return dest;
}

void error_data_add(ParseError *data, size_t token_index,
                    TokenKind expected_kind) {
  if (data->first_unparsed_token > token_index)
    return;
  if (data->first_unparsed_token < token_index) {
    data->num_expected = 0;
    data->first_unparsed_token = token_index;
  }
  for (size_t j = 0; j < data->num_expected; j++) {
    if (expected_kind == data->expected[j]) {
      return;
    }
  }
  if (data->num_expected >= MAX_NUM_EXPECTED) {
    panicf("error_data->num_expected exceeds MAX_NUM_EXPECTED\n");
  }
  data->expected[data->num_expected] = expected_kind;
  data->num_expected += 1;
}

#define _ERROR_DATA_ADD_1(a) error_data_add(error_data, *token_index, a);
#define _ERROR_DATA_ADD_2(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_1(__VA_ARGS__)
#define _ERROR_DATA_ADD_3(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_2(__VA_ARGS__)
#define _ERROR_DATA_ADD_4(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_3(__VA_ARGS__)
#define _ERROR_DATA_ADD_5(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_4(__VA_ARGS__)
#define _ERROR_DATA_ADD_6(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_5(__VA_ARGS__)
#define _ERROR_DATA_ADD_7(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_6(__VA_ARGS__)
#define _ERROR_DATA_ADD_8(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_7(__VA_ARGS__)
#define _ERROR_DATA_ADD_9(a, ...)                                              \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_8(__VA_ARGS__)
#define _ERROR_DATA_ADD_10(a, ...)                                             \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_9(__VA_ARGS__)
#define _ERROR_DATA_ADD_11(a, ...)                                             \
  _ERROR_DATA_ADD_1(a) _ERROR_DATA_ADD_10(__VA_ARGS__)

#define ERROR_DATA_ADD(...)                                                    \
  {_GET_MACRO(__VA_ARGS__, _ERROR_DATA_ADD_11, _ERROR_DATA_ADD_10,             \
              _ERROR_DATA_ADD_9, _ERROR_DATA_ADD_8, _ERROR_DATA_ADD_7,         \
              _ERROR_DATA_ADD_6, _ERROR_DATA_ADD_5, _ERROR_DATA_ADD_4,         \
              _ERROR_DATA_ADD_3, _ERROR_DATA_ADD_2,                            \
              _ERROR_DATA_ADD_1)(__VA_ARGS__)}

#define YIELD($kind, ...)                                                      \
  *out =                                                                       \
      yield(allocator,                                                         \
            (Node){.span = span, .kind = NODE_##$kind, .$kind = __VA_ARGS__});

#define RETURN($kind, ...)                                                     \
  YIELD($kind, __VA_ARGS__);                                                   \
  OK;

#ifdef DEBUG_PARSER
int log_indent = 0;

// prints token index, number of nodes, depth
#define LOG(format, ...)                                                       \
  eprintf("%-3zu %-3zu %-2d %*.*s" format "\n", *token_index,                  \
          allocator->length, log_indent, log_indent, log_indent,               \
          " " __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ENTER                                                              \
  {                                                                            \
    LOG("Enter: %s", __parser_name);                                           \
    log_indent++;                                                              \
  }

#define LOG_EXIT(status)                                                       \
  {                                                                            \
    log_indent--;                                                              \
    LOG("Exit " status ": %s", __parser_name);                                 \
  }

#define BEGIN_LOG(name)                                                        \
  const char *__parser_name = #name;                                           \
  LOG_ENTER;

#else
#define LOG(format, ...)
#define LOG_ENTER
#define LOG_EXIT(status)
#define BEGIN_LOG(name)
#endif

#define GENERAL_PARAMS                                                         \
  Allocator *allocator, TokenStream stream, size_t *token_index,               \
      ParseError *error_data
#define PARSER_PROTOTYPE($name) bool parse_##$name(GENERAL_PARAMS, Node **out)

#define BEGIN($name)                                                           \
  BEGIN_LOG($name);                                                            \
  size_t start_token_index = *token_index;                                     \
  size_t start_allocator_length = allocator->length;                           \
  Token token = PEEK;                                                          \
  Span span = (Span){.offset = token.offset, .length = 0};                     \
  UNUSED(span);                                                                \
  UNUSED(start_allocator_length);                                              \
  UNUSED(start_token_index);

#define OK                                                                     \
  {                                                                            \
    LOG_EXIT("OK");                                                            \
    return true;                                                               \
  }

#define FAIL                                                                   \
  {                                                                            \
    *token_index = start_token_index;                                          \
    la_shrink(allocator, start_allocator_length);                              \
    LOG_EXIT("FAIL");                                                          \
    return false;                                                              \
  }

#define PARSER($name, ...)                                                     \
  PARSER_PROTOTYPE($name) {                                                    \
    BEGIN(name);                                                               \
    __VA_ARGS__;                                                               \
  }

#define EXTEND_SPAN($span)                                                     \
  span.length = ($span).offset + ($span).length - span.offset

#define JOIN_SPANS($left, $right) join_spans($left->span, $right->span)

#define CONSUME                                                                \
  {                                                                            \
    LOG("Consume: `%s`", token_display_name(token.kind));                      \
    *token_index += 1;                                                         \
    EXTEND_SPAN(token);                                                        \
  }

#define PEEK get_token(stream, *token_index)

#define UNEXPECTED($expected...)                                               \
  {                                                                            \
    LOG("Expected: " #$expected " found %s", token_name(token.kind));          \
    ERROR_DATA_ADD($expected);                                                 \
  }

// Tries to match a token, running $on_fail on failure.
#define TRY_TOKEN($on_fail, $expected...)                                      \
  token = PEEK;                                                                \
  if (!IS_ONE_OF(token.kind, $expected)) {                                     \
    UNEXPECTED($expected);                                                     \
    $on_fail;                                                                  \
  }                                                                            \
  CONSUME;

#define EXPECT($expected...) TRY_TOKEN(FAIL, $expected)

#define EXPECT_OP($ops...)                                                     \
  EXPECT($ops);                                                                \
  TokenKind op = token.kind;

#define TRY_OP($on_fail, $ops...)                                              \
  TRY_TOKEN($on_fail, $ops);                                                   \
  TokenKind op = token.kind;

#define PARSE($name, $node_ptr)                                                \
  parse_##$name(allocator, stream, token_index, error_data, $node_ptr)

// Try to parse and return on failure.
#define MUST_PARSE($name, $node)                                               \
  Node *$node = NULL;                                                          \
  if (!PARSE($name, &$node))                                                   \
    FAIL;

// Try to parse and continue regardless of failure or success.
#define MAY_PARSE($name, $node)                                                \
  Node *$node = NULL;                                                          \
  PARSE($name, &$node);

// Try to parse and return on success.
#define TRY_PARSE($name)                                                       \
  if (PARSE($name, out))                                                       \
    OK;

#define ZERO_OR_MORE($element, $nodes)                                         \
  NodeArray $nodes = {0};                                                      \
  {                                                                            \
    Node *node = NULL;                                                         \
    while (PARSE($element, &node)) {                                           \
      assert(node != NULL);                                                    \
      $nodes.data = la_realloc(allocator, $nodes.data,                         \
                               sizeof(ASTNode *) * ($nodes.length + 1));       \
      $nodes.data[$nodes.length] = node;                                       \
      $nodes.length++;                                                         \
    }                                                                          \
    if ($nodes.length > 0)                                                     \
      EXTEND_SPAN($nodes.data[$nodes.length - 1]->span);                       \
  }

#define ZERO_OR_MORE_SEPARATED($element, $nodes, $separators...)               \
  NodeArray $nodes = {0};                                                      \
  {                                                                            \
    Node *node = NULL;                                                         \
    while (PARSE($element, &node)) {                                           \
      $nodes.data = la_realloc(allocator, $nodes.data,                         \
                               sizeof(ASTNode *) * ($nodes.length + 1));       \
      $nodes.data[$nodes.length] = node;                                       \
      $nodes.length++;                                                         \
      /*NOTE: should EXTEND_SPAN not be used for every node parsed normally as \
       * well? */                                                              \
      EXTEND_SPAN(node->span);                                                 \
      TRY_TOKEN(break, $separators);                                           \
    }                                                                          \
  }

PARSER_PROTOTYPE(expr);
PARSER_PROTOTYPE(stmt);

bool parse_block(GENERAL_PARAMS, Span *out_span, NodeArray *out_stmts) {
  BEGIN(block);
  EXPECT(tLBRACE);

  ZERO_OR_MORE(stmt, stmts);
  *out_stmts = stmts;

  EXPECT(tRBRACE)
  *out_span = span;
  OK; // FIXME: silent pointer->bool conversion bug
}

#define MUST_PARSE_BLOCK($name)                                                \
  NodeArray $name##_stmts = {0};                                               \
  {                                                                            \
    Span block_span = {0};                                                     \
    if (!parse_block(allocator, stream, token_index, error_data, &block_span,  \
                     &$name##_stmts))                                          \
      FAIL;                                                                    \
    EXTEND_SPAN(block_span);                                                   \
  }

PARSER(name, {
  EXPECT(tIDENT);
  RETURN(name, {.name = {.text = token.text, .length = token.length},
                .value_ptr = NULL});
});

PARSER(integer, {
  EXPECT(tNUMBER);
  RETURN(integer, {.text = token.text, .length = token.length});
});

PARSER(string, {
  EXPECT(tSTRING);
  RETURN(string, {.text = token.text + 1, .length = token.length - 2});
});

PARSER(function_call, {
  MUST_PARSE(name, function);
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(expr, args, tCOMMA);
  EXPECT(tRPAREN);
  RETURN(function_call, {.function = function, .args = args});
});

PARSER(boolean, {
  EXPECT(tTRUE, tFALSE);
  RETURN(boolean, {.text = token.text, .length = token.length});
});

PARSER(paren_expr, {
  EXPECT(tLPAREN);
  MUST_PARSE(expr, expr);
  *out = expr;
  EXPECT(tRPAREN);
  OK;
});

PARSER(primary, {
  // `function_call` must be tried before `name` because `name` is a prefix of
  // `function_call`
  TRY_PARSE(function_call);
  TRY_PARSE(name);
  TRY_PARSE(integer);
  TRY_PARSE(boolean);
  TRY_PARSE(string);
  TRY_PARSE(paren_expr);
  FAIL;
})

PARSER(unary, {
  TRY_PARSE(primary);

  EXPECT_OP(tMINUS);
  MUST_PARSE(primary, expr);
  RETURN(unary, {.op = op, .expr = expr});
});

void fix_precedence(Node *node) {
  assert(node->kind == aBINARY);
  __auto_type binary = &node->binary;
  assert(binary->left->kind != aBINARY);

  if (binary->right->kind == aBINARY) {
    __auto_type right = &binary->right->binary;

    if (!right->has_parens &&
        token_precedence(right->op) < token_precedence(binary->op)) {
      Node *binary_right = binary->right;
      // performs a left-rotation on the tree
      // NOTE: this does not preserve order of nodes
      binary->right = right->left;
      right->left = node;
      SWAP(node, binary_right);
    }
  }
}

PARSER(binary, {
  MUST_PARSE(unary, left);
  TRY_OP(
      {
        *out = left;
        OK;
      },
      tPLUS, tMINUS, tSTAR, tSLASH, tLT, tGT, tLT_EQ, tGT_EQ, tEQ_EQ, tAMP_AMP,
      tBAR_BAR);
  MUST_PARSE(expr, right);

  YIELD(binary, {.op = op, .has_parens = false, .left = left, .right = right});
  fix_precedence(allocator->data[allocator->length - 1]);
  OK;
});

PARSER(param, {
  MUST_PARSE(name, name);
  EXPECT(tCOL);
  MUST_PARSE(name, type);
  RETURN(param, {.name = name, .type = type});
});

PARSER(return_type, {
  EXPECT(tRARROW);
  TRY_PARSE(name);
  FAIL;
});

PARSER(function, {
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(param, params, tCOMMA);
  EXPECT(tRPAREN);
  MAY_PARSE(return_type, return_type);
  MUST_PARSE_BLOCK(body);
  RETURN(function,
         {.params = params, .return_type = return_type, .stmts = body_stmts});
});

PARSER(expr, {
  TRY_PARSE(binary);
  FAIL;
});

PARSER(expr_stmt, {
  MUST_PARSE(expr, expr);
  *out = expr;
  EXPECT(tSEMI);
  OK;
});

PARSER_PROTOTYPE(decl);

PARSER(assign, {
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ, tPLUS_EQ, tMINUS_EQ, tSTAR_EQ,
            tSLASH_EQ); // TODO: handle tPLUS_EQ (and co.) operators
  MUST_PARSE(expr_stmt, expr);
  RETURN(assign, {.op = op, .target = name, .expr = expr});
});

PARSER(return_stmt, {
  EXPECT(tRETURN);
  MUST_PARSE(expr_stmt, expr);
  RETURN(return_stmt, {.expr = expr});
});

PARSER(if_stmt, {
  EXPECT(tIF);
  MUST_PARSE(expr_stmt, cond);
  MUST_PARSE_BLOCK(then);
  TRY_TOKEN(
      { RETURN(if_stmt, {.cond = cond, .then_stmts = then_stmts}); }, tELSE);
  MUST_PARSE_BLOCK(else);
  RETURN(if_stmt,
         {.cond = cond, .then_stmts = then_stmts, .else_stmts = else_stmts});
});

PARSER(while_loop, {
  EXPECT(tWHILE);
  MUST_PARSE(expr_stmt, cond);
  MUST_PARSE_BLOCK(body);

  RETURN(while_loop, {.cond = cond, .stmts = body_stmts});
});

PARSER(for_loop, {
  EXPECT(tFOR);
  MUST_PARSE(decl, init);
  MUST_PARSE(expr_stmt, cond);
  MUST_PARSE(assign, step);
  MUST_PARSE_BLOCK(body);

  RETURN(for_loop,
         {.init = init, .cond = cond, .step = step, .stmts = body_stmts});
});

PARSER(field, {
  MUST_PARSE(name, name);
  EXPECT(tCOL);
  MUST_PARSE(name, type);
  EXPECT(tSEMI);
  RETURN(field, {.name = name, .type = type});
});

PARSER(_struct, {
  EXPECT(tSTRUCT);
  EXPECT(tLBRACE);
  ZERO_OR_MORE(field, fields);
  EXPECT(tRBRACE);
  RETURN(_struct, {.fields = fields});
});

PARSER(decl_expr, {
  TRY_PARSE(function);
  TRY_PARSE(_struct);
  TRY_PARSE(expr_stmt);
  OK;
});

PARSER(decl, {
  MUST_PARSE(name, name);

  EXPECT(tCOL_COL, tCOL_EQ);
  bool is_constant = token.kind == tCOL_COL;

  MUST_PARSE(decl_expr, expr);

  RETURN(decl, {.is_constant = is_constant, .name = name, .expr = expr});
});

PARSER(stmt, {
  TRY_PARSE(expr_stmt);
  TRY_PARSE(if_stmt);
  TRY_PARSE(while_loop);
  TRY_PARSE(for_loop);
  TRY_PARSE(assign);
  TRY_PARSE(return_stmt);
  TRY_PARSE(decl);
  FAIL;
});

PARSER(module, {
  ZERO_OR_MORE(decl, decls);
  RETURN(module, {.decls = decls});
});

void print_parse_error(const char *source, TokenStream stream,
                       ParseError error_data) {
  Token token;
  if (error_data.first_unparsed_token == stream.length) {
    eprintf("Parse error: unexpected token EOF. "
            "Expected one of [");
  } else {
    token = stream.data[error_data.first_unparsed_token];
    eprintf("Parse error: unexpected token `%.*s`. Expected one of [",
            (int)token.length, token.text);
  }
  for (size_t i = 0; i < error_data.num_expected; i++) {
    TokenKind expected = error_data.expected[i];
    if (i > 0)
      eprintf(", ");
    eprintf("`%s`", token_display_name(expected));
  }
  eprintf("].\n");

  if (error_data.first_unparsed_token != stream.length) {
    OffsetInfo oi = get_offset_info(source, token.offset);
    eprintf(">>> line %zu, column %zu\n", oi.line_number, oi.column_number);
    eprintf(" | %.*s\n", (int)oi.line_length, oi.line);
    eprintf(" | %*s", (int)oi.column_number, " ");
    for (size_t i = 0; i < token.length; i++)
      eprintf("~");
    eprintf("\n");
  }
}

bool parse_token_stream(TokenStream stream, AST *out_ast,
                        ParseError *error_data) {
  *out_ast = (AST){0};
  *error_data = (ParseError){0};
  size_t token_index = 0;
  bool result = parse_module(&out_ast->list_allocator, stream, &token_index,
                             error_data, &out_ast->head);
  if (token_index < stream.length) {
    return false;
  }
  return result;
}
