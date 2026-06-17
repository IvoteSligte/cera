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

typedef struct {
  Allocator *allocator;
  TokenStream stream;
  ParseError *error_data;
} State;

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
    panicf("error_data->num_expected exceeds MAX_NUM_EXPECTED");
  }
  data->expected[data->num_expected] = expected_kind;
  data->num_expected += 1;
}

#define _ERROR_DATA_ADD_1(a) error_data_add(state->error_data, *token_index, a);
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
      yield(state->allocator,                                                  \
            (Node){.span = span, .kind = NODE_##$kind, .$kind = __VA_ARGS__});

#define RETURN($kind, ...)                                                     \
  YIELD($kind, __VA_ARGS__);                                                   \
  OK;

#ifdef DEBUG_PARSER
int log_indent = 0;

// prints token index, number of nodes, depth
#define LOG(format, ...)                                                       \
  eprintf("%-3zu %-3zu %-2d %*.*s" format "\n", *token_index,                  \
          state->allocator->length, log_indent, log_indent, log_indent,        \
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

#define PARSER_SIGNATURE($name)                                                \
  bool parse_##$name(State *state, size_t *token_index, Node **out)

#define BEGIN($name)                                                           \
  BEGIN_LOG($name);                                                            \
  size_t start_token_index = *token_index;                                     \
  size_t start_allocator_length = state->allocator->length;                    \
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
    la_shrink(state->allocator, start_allocator_length);                       \
    LOG_EXIT("FAIL");                                                          \
    return false;                                                              \
  }

#define PARSER($name, ...)                                                     \
  PARSER_SIGNATURE($name) {                                                    \
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

#define PEEK get_token(state->stream, *token_index)

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

#define PARSE($name, $node_ptr) parse_##$name(state, token_index, $node_ptr)

// Try to parse and return on failure.
#define MUST_PARSE($parser, $node)                                             \
  Node *$node = NULL;                                                          \
  if (!PARSE($parser, &$node))                                                 \
    FAIL;

// Try to parse and continue regardless of failure or success.
#define MAY_PARSE($parser, $node)                                              \
  Node *$node = NULL;                                                          \
  PARSE($parser, &$node);

// Try to parse and return on success.
#define TRY_PARSE($parser)                                                     \
  if (PARSE($parser, out))                                                     \
    OK;

#define ZERO_OR_MORE($element, $nodes)                                         \
  NodeArray $nodes = {0};                                                      \
  {                                                                            \
    Node *node = NULL;                                                         \
    while (PARSE($element, &node)) {                                           \
      assert(node != NULL);                                                    \
      $nodes.data = la_realloc(state->allocator, $nodes.data,                  \
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
      $nodes.data = la_realloc(state->allocator, $nodes.data,                  \
                               sizeof(ASTNode *) * ($nodes.length + 1));       \
      $nodes.data[$nodes.length] = node;                                       \
      $nodes.length++;                                                         \
      /* NOTE: should EXTEND_SPAN not be used for every node parsed normally   \
       * as well? */                                                           \
      EXTEND_SPAN(node->span);                                                 \
      TRY_TOKEN(break, $separators);                                           \
    }                                                                          \
  }

PARSER_SIGNATURE(expr);
PARSER_SIGNATURE(stmt);

bool parse_block(State *state, size_t *token_index, Span *out_span,
                 NodeArray *out_stmts) {
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
    if (!parse_block(state, token_index, &block_span, &$name##_stmts))         \
      FAIL;                                                                    \
    EXTEND_SPAN(block_span);                                                   \
  }

PARSER(name, {
  EXPECT(tIDENT);
  RETURN(name, {.name = {.text = token.text, .length = token.length}});
});

PARSER(integer, {
  EXPECT(tNUMBER);
  RETURN(integer, {.text = token.text, .length = token.length});
});

PARSER(string, {
  EXPECT(tSTRING);
  RETURN(string, {.text = token.text + 1, .length = token.length - 2});
});

PARSER(func_call, {
  MUST_PARSE(name, function);
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(expr, args, tCOMMA);
  EXPECT(tRPAREN);
  RETURN(func_call, {.function = function, .args = args});
});

PARSER(field_inst, {
  MUST_PARSE(name, name);
  EXPECT(tCOL);
  MUST_PARSE(expr, expr);
  RETURN(field_inst, {.name = name, .expr = expr});
});

PARSER(struct_inst, {
  MUST_PARSE(name, type);
  EXPECT(tDOT);
  EXPECT(tLBRACE);
  ZERO_OR_MORE_SEPARATED(field_inst, fields, tCOMMA);
  EXPECT(tRBRACE);
  RETURN(struct_inst, {.type = type, .fields = fields});
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
  // `func_call`, `struct_inst`, and `member` must be tried before `name`
  // because `name` is their prefix
  TRY_PARSE(func_call);
  TRY_PARSE(struct_inst);
  TRY_PARSE(name);
  TRY_PARSE(integer);
  TRY_PARSE(boolean);
  TRY_PARSE(string);
  TRY_PARSE(paren_expr);
  FAIL;
});

PARSER(member, {
  MAY_PARSE(primary, expr);

  // TODO: nested member access 'a.b.c' -> '(a.b).c'
  TRY_TOKEN(
      {
        if (expr == NULL) {
          FAIL;
        } else {
          *out = expr;
          OK;
        }
      },
      tDOT);
  MUST_PARSE(name, name);
  RETURN(member, {.expr = expr, .name = name});
});

PARSER(unary, {
  TRY_PARSE(member);

  EXPECT_OP(tMINUS);
  MUST_PARSE(member, expr);
  RETURN(unary, {.op = op, .expr = expr});
});

// Binary operators are initially always parsed right-associatively
// as `a * [b + [c == d]]`, regardless of the true operator precedence
// and associativity.
// This function fixes the tree structure to account for precedence.
// All operators are assumed to be left-associative.
// Note that this function is already called on the right-hand binary
// expression, so the actual input would be `a * [[b + c] == d]`.
// This is first mapped to `[a * [b + c]] == d` and then a recursive call
// maps it to the desired `[[a * b] + c] == d`.
void fix_precedence(Node *node) {
  assert(node->kind == aBINARY);
  __auto_type binary = &node->binary;
  assert(binary->left->kind != aBINARY);

  if (binary->right->kind != aBINARY) {
    return;
  }
  __auto_type right = &binary->right->binary;
  if (right->has_parens ||
      token_precedence(right->op) > token_precedence(binary->op)) {
    return;
  }
  // Rewrites `a * [b + c]` -> `[a * b] + c`
  Node *children[3] = {binary->left, right->left, right->right};
  Node *new_left = binary->right;
  SWAP(&binary->op, &right->op);
  {
    binary->left = new_left;
    new_left->binary.left = children[0];
    new_left->binary.right = children[1];
  }
  binary->right = children[2];

  fix_precedence(binary->left);
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
  fix_precedence(*out);
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

PARSER(func_decl, {
  EXPECT(tFUNC);
  MUST_PARSE(name, name);
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(param, params, tCOMMA);
  EXPECT(tRPAREN);
  MAY_PARSE(return_type, return_type);
  MUST_PARSE_BLOCK(body);
  RETURN(func_decl, {.name = name,
                     .params = params,
                     .return_type = return_type,
                     .stmts = body_stmts});
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

PARSER_SIGNATURE(var_decl);

PARSER(assign, {
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ, tPLUS_EQ, tMINUS_EQ, tSTAR_EQ, tSLASH_EQ);
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
  MUST_PARSE(var_decl, init);
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

PARSER(struct_decl, {
  EXPECT(tSTRUCT);
  MUST_PARSE(name, name);
  EXPECT(tLBRACE);
  ZERO_OR_MORE(field, fields);
  EXPECT(tRBRACE);
  RETURN(struct_decl, {.name = name, .fields = fields});
});

PARSER(var_decl, {
  MUST_PARSE(name, name);

  EXPECT(tCOL_COL, tCOL_EQ);
  bool is_constant = token.kind == tCOL_COL;

  MUST_PARSE(expr, expr);
  EXPECT(tSEMI);

  RETURN(var_decl, {.is_constant = is_constant, .name = name, .expr = expr});
});

PARSER(stmt, {
  TRY_PARSE(expr_stmt);
  TRY_PARSE(if_stmt);
  TRY_PARSE(while_loop);
  TRY_PARSE(for_loop);
  TRY_PARSE(assign);
  TRY_PARSE(return_stmt);
  TRY_PARSE(var_decl);
  FAIL;
});

PARSER(decl, {
  TRY_PARSE(func_decl);
  TRY_PARSE(struct_decl);
  TRY_PARSE(var_decl);
  FAIL;
});

PARSER(module, {
  ZERO_OR_MORE(decl, decls);
  RETURN(module, {.decls = decls});
});

void print_parse_error(const char *source, TokenStream stream,
                       ParseError error_data) {
  Token token = {0};
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

void get_parse_error_info(const char *source, TokenStream stream,
                          ParseError error_data, char **out_message,
                          size_t *out_line, size_t *out_column) {
  size_t offset = 0;
  if (error_data.first_unparsed_token == stream.length) {
    *out_message = strdup("unexpected EOF");
    offset = strlen(source);
  } else {
    *out_message = strdup("unexpected token");
    offset = stream.data[error_data.first_unparsed_token].offset;
  }
  *out_message = strdup("parse error");
  OffsetInfo oi = get_offset_info(source, offset);
  *out_line = oi.line_number;
  *out_column = oi.column_number;
}

bool parse_token_stream(TokenStream stream, AST *out_ast,
                        ParseError *error_data) {
  *out_ast = (AST){0};
  *error_data = (ParseError){0};
  State state = {.allocator = &out_ast->list_allocator,
                 .stream = stream,
                 .error_data = error_data};
  size_t token_index = 0;
  bool result = parse_module(&state, &token_index, &out_ast->head);
  if (token_index < stream.length) {
    return false;
  }
  return result;
}
