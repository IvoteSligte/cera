#include "parser.h"
#include "alloc.h"
#include "ast.h"
#include "ast_macro.h"

#include <stdarg.h>

typedef ListAllocator Allocator;

void yield(Allocator *allocator, ASTNode node) {
  ASTNode *dest = la_calloc(allocator, sizeof(ASTNode));
  *dest = node;
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

#define ERROR_DATA_ADD(...)                                                    \
  {_GET_MACRO(__VA_ARGS__, _ERROR_DATA_ADD_5, _ERROR_DATA_ADD_4,               \
              _ERROR_DATA_ADD_3, _ERROR_DATA_ADD_2,                            \
              _ERROR_DATA_ADD_1)(__VA_ARGS__)}

#define YIELD(KIND, $kind, ...)                                                \
  yield(allocator, (ASTNode){.span = span,                                     \
                             .next_sibling = NULL,                             \
                             .kind = KIND,                                     \
                             .$kind = __VA_ARGS__})

#define RETURN($kind, ...)                                                     \
  YIELD(UPPER_##$kind, $kind, __VA_ARGS__);                                    \
  OK;

int log_indent = 0;

#define LOG(format, ...)                                                       \
  eprintf("%-4zu %-3zu %*.*s" format "\n", *token_index, allocator->length,    \
          log_indent, log_indent, " " __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ENTER                                                              \
  {                                                                            \
    LOG("Enter: %s", parser_name);                                             \
    log_indent++;                                                              \
  }
#define LOG_EXIT(status)                                                       \
  {                                                                            \
    log_indent--;                                                              \
    LOG("Exit " status ": %s", parser_name);                                   \
  }

#define PARSER_PARAMS                                                          \
  Allocator *allocator, TokenStream stream, size_t *token_index,               \
      ParseError *error_data
#define PARSER_PROTOTYPE(name) bool parse_##name(PARSER_PARAMS)

#define BEGIN(name)                                                            \
  const char *parser_name = #name;                                             \
  LOG_ENTER;                                                                   \
  size_t start_token_index = *token_index;                                     \
  size_t start_allocator_length = allocator->length;                           \
  Token token;                                                                 \
  if (!peek_token(stream, *token_index, &token))                               \
    FAIL;                                                                      \
  Span span = (Span){.offset = token.offset, .length = 0};

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

#define PARSER(name, ...)                                                      \
  PARSER_PROTOTYPE(name) {                                                     \
    BEGIN(name);                                                               \
    __VA_ARGS__;                                                               \
  }

#define EXTEND_SPAN($span)                                                     \
  span.length = ($span).offset + ($span).length - span.offset

#define JOIN_SPANS(left, right) join_spans(left->span, right->span)

#define CONSUME                                                                \
  {                                                                            \
    LOG("Consume: `%s`", token_display_name(token.kind));                      \
    *token_index += 1;                                                         \
    EXTEND_SPAN(token);                                                        \
  }

#define EXPECT(...)                                                            \
  {                                                                            \
    if (!peek_token(stream, *token_index, &token)) {                           \
      LOG("Expected: " #__VA_ARGS__ " found EOF");                             \
      ERROR_DATA_ADD(__VA_ARGS__);                                             \
      FAIL;                                                                    \
    }                                                                          \
    if (!IS_ONE_OF(token.kind, __VA_ARGS__)) {                                 \
      LOG("Expected: " #__VA_ARGS__ " found %s", token_name(token.kind));      \
      ERROR_DATA_ADD(__VA_ARGS__);                                             \
      FAIL;                                                                    \
    }                                                                          \
    CONSUME;                                                                   \
  }

#define EXPECT_OP(...)                                                         \
  EXPECT(__VA_ARGS__);                                                         \
  TokenKind op = token.kind;

#define PARSE(name) parse_##name(allocator, stream, token_index, error_data)

// Try to parse and return on failure.
#define MUST_PARSE(name, node)                                                 \
  if (!PARSE(name))                                                            \
    FAIL;                                                                      \
  ASTNode *node = allocator->data[allocator->length - 1];

// Try to parse and continue on success.
// Declares has_##out indicating whether it succeeded.
// TODO: is it possible to differentiate between 'not found' and error?
#define MAY_PARSE(name, node)                                                  \
  ASTNode *node = PARSE(name) ? allocator->data[allocator->length] : NULL;

// Try to parse and return on success.
#define TRY_PARSE(name)                                                        \
  if (PARSE(name))                                                             \
    OK;

#define ZERO_OR_MORE(element, node)                                            \
  ASTNode *first_##node = NULL;                                                \
  {                                                                            \
    ASTNode *last_##node = NULL;                                               \
    while (PARSE(element)) {                                                   \
      last_##node = allocator->data[allocator->length - 1];                    \
      if (first_##node == NULL) {                                              \
        first_##node = last_##node;                                            \
      } else {                                                                 \
        last_##node->next_sibling = allocator->data[allocator->length - 1];    \
      }                                                                        \
    }                                                                          \
    if (last_##node != NULL)                                                   \
      EXTEND_SPAN(last_##node->span);                                          \
  }

#define ZERO_OR_MORE_SEPARATED(element, node, ...)                             \
  ASTNode *first_##node = NULL;                                                \
  size_t num_##node##s = 0;                                                    \
  {                                                                            \
    ASTNode *last_##node = NULL;                                               \
    while (PARSE(element)) {                                                   \
      last_##node = allocator->data[allocator->length - 1];                    \
      num_##node##s++;                                                         \
      if (first_##node == NULL) {                                              \
        first_##node = last_##node;                                            \
      } else {                                                                 \
        last_##node->next_sibling = allocator->data[allocator->length - 1];    \
      }                                                                        \
      if (!peek_token(stream, *token_index, &token) ||                         \
          !IS_ONE_OF(token.kind, __VA_ARGS__)) {                               \
        break;                                                                 \
      }                                                                        \
      *token_index += 1;                                                       \
      EXTEND_SPAN(token);                                                      \
    }                                                                          \
    if (last_##node != NULL)                                                   \
      EXTEND_SPAN(last_##node->span);                                          \
  }

PARSER_PROTOTYPE(expr);
PARSER_PROTOTYPE(stmt);

bool parse_block(PARSER_PARAMS, ASTNode **out_first_stmt, Span *out_span) {
  BEGIN(block);
  EXPECT(tLBRACE);

  ZERO_OR_MORE(stmt, stmt);
  *out_first_stmt = first_stmt;

  EXPECT(tRBRACE)
  *out_span = span;
  OK;
}

#define MUST_PARSE_BLOCK                                                       \
  ASTNode *first_stmt = NULL;                                                  \
  Span block_span = {0};                                                       \
  if (!parse_block(allocator, stream, token_index, error_data, &first_stmt,    \
                   &block_span))                                               \
    FAIL;                                                                      \
  EXTEND_SPAN(block_span);

PARSER(name, {
  EXPECT(tIDENT);
  RETURN(name, {token.text, token.length});
});

PARSER(integer, {
  EXPECT(tNUMBER);
  RETURN(integer, {token.text, token.length});
});

PARSER(string, {
  EXPECT(tSTRING);
  RETURN(string, {token.text + 1, token.length - 2});
});

PARSER(function_call, {
  MUST_PARSE(name, function);
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(expr, arg, tCOMMA);
  EXPECT(tRPAREN);
  RETURN(function_call, {.function = function, .args = first_arg});
});

PARSER(paren_expr, {
  EXPECT(tLPAREN);
  MUST_PARSE(expr, expr);
  EXPECT(tRPAREN);
  OK;
});

PARSER(primary, {
  // function_call must be tried before name because name is a prefix of
  // function_call
  TRY_PARSE(function_call);
  TRY_PARSE(name);
  TRY_PARSE(integer);
  TRY_PARSE(string);
  FAIL;
})

PARSER(unary, {
  TRY_PARSE(primary);

  EXPECT_OP(tMINUS);
  MUST_PARSE(primary, expr);
  RETURN(unary, {.op = op, .expr = expr});
});

void fix_precedence(ASTNode *node) {
  assert(node->kind == BINARY);
  __auto_type binary = &node->binary;
  assert(binary->left->kind != BINARY);

  if (binary->right->kind == BINARY) {
    __auto_type right = &binary->right->binary;

    if (!right->has_parens &&
        token_precedence(right->op) < token_precedence(binary->op)) {
      ASTNode *binary_right = binary->right;
      // performs a left-rotation on the tree
      // NOTE: this does not preserve order of nodes
      binary->right = right->left;
      right->left = node;
      SWAP(node, binary_right);
    }
  }
}

PARSER(binary, {
  TRY_PARSE(unary);

  MUST_PARSE(unary, left);
  EXPECT_OP(tPLUS, tMINUS, tSTAR, tSLASH);
  MUST_PARSE(expr, right);

  YIELD(BINARY, binary,
        {.op = op, .has_parens = false, .left = left, .right = right});
  fix_precedence(allocator->data[allocator->length - 1]);
  OK;
});

PARSER(param, {
  MUST_PARSE(name, name);
  EXPECT(tCOL);
  MUST_PARSE(name, type);
  RETURN(param, {.name = name, .type = type});
});

PARSER(function, {
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(param, param, tCOMMA);
  EXPECT(tRPAREN);
  MAY_PARSE(name, return_type);
  MUST_PARSE_BLOCK;
  RETURN(function, {.params = first_param,
                    .num_params = num_params,
                    .return_type = return_type,
                    .stmts = first_stmt});
});

PARSER(expr, {
  TRY_PARSE(binary);
  FAIL;
});

PARSER(expr_stmt, {
  MUST_PARSE(expr, expr);
  EXPECT(tSEMI);
  OK;
});

PARSER_PROTOTYPE(declaration);

PARSER(assign, {
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ);
  MUST_PARSE(expr, value);
  EXPECT(tSEMI);
  RETURN(assign, {.op = op, .name = name, .value = value});
});

PARSER(return_stmt, {
  EXPECT(tRETURN);
  MUST_PARSE(expr, expr);
  EXPECT(tSEMI);
  RETURN(return_stmt, {.expr = expr});
});

PARSER(for_loop, {
  MUST_PARSE(declaration, init);
  MUST_PARSE(expr, cond);
  MUST_PARSE(assign, step);
  MUST_PARSE_BLOCK;

  RETURN(for_loop,
         {.init = init, .cond = cond, .step = step, .stmts = first_stmt});
});

PARSER(declaration_value, {
  TRY_PARSE(expr);
  TRY_PARSE(function);
  OK;
});

PARSER(declaration, {
  MUST_PARSE(name, name);

  EXPECT(tCOLCOL, tCOLEQ);
  bool is_constant = token.kind == tCOLCOL;

  MUST_PARSE(declaration_value, value);
  EXPECT(tSEMI);

  RETURN(declaration,
         {.is_constant = is_constant, .name = name, .value = value});
});

PARSER(stmt, {
  TRY_PARSE(expr_stmt);
  TRY_PARSE(for_loop);
  TRY_PARSE(assign);
  TRY_PARSE(return_stmt);
  TRY_PARSE(declaration);
  FAIL;
});

PARSER(module, {
  ZERO_OR_MORE(declaration, declaration);
  RETURN(module, {.declarations = first_declaration});
});

typedef struct {
  const char *line;
  int line_length;
  int column_number;
  size_t line_number;
} OffsetInfo;

OffsetInfo get_offset_info(const char *source, size_t offset) {
  OffsetInfo info = {0};
  info.line = source;

  for (size_t i = 0; i < offset; i++) {
    if (source[i] == '\n') {
      info.line = &source[i];
      info.line_number++;
    } else {
      info.column_number++;
    }
  }
  info.line_length = info.column_number;
  for (size_t i = offset; source[i] != '\n' && source[i] != '\0'; i++) {
    info.line_length++;
  }
  return info;
}

void print_parse_error(const char *source, TokenStream stream,
                       ParseError error_data) {
  Token token;
  if (error_data.first_unparsed_token == stream.length) {
    eprintf("Parse error: unexpected token EOF. "
            "Expected one of [");
  } else {
    token = stream.data[error_data.first_unparsed_token];
    eprintf("Parse error: unexpected token `%.*s` at offset %zu. "
            "Expected one of [",
            (int)token.length, token.text, token.offset);
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
    eprintf(">>> line %zu, column %d\n", oi.line_number, oi.column_number);
    eprintf(" | %.*s\n", oi.line_length, oi.line);
    eprintf(" | %*s", oi.column_number, " ");
    for (size_t i = 0; i < token.length; i++)
      eprintf("~");
    eprintf("\n");
  }
}

bool parse(TokenStream stream, AST *out_ast, ParseError *error_data) {
  Allocator allocator = {0};
  size_t token_index = 0;
  *error_data = (ParseError){0};
  bool result = parse_module(&allocator, stream, &token_index, error_data);
  *out_ast = (AST){.allocator = allocator,
                   .head = allocator.data[allocator.length - 1]};
  if (token_index < stream.length) {
    return false;
  }
  return result;
}
