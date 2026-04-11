#include "parser.h"
#include "ast.h"
#include "ast_macro.h"

#include <stdarg.h>

// FIXME: spaces not being skipped

typedef struct {
  ASTNode *data;
  size_t length;
} ASTNodeArray;

void yield(ASTNodeArray *node_array, size_t *node_index, ASTNode node) {
  assert(node.tree_size > 0);
  if (*node_index >= node_array->length) {
    node_array->data =
        realloc(node_array->data, sizeof(ASTNode) * (*node_index + 1));
  }
  node_array->data[*node_index] = node;
  *node_index += 1;
}

#define MAX_NUM_EXPECTED 20

typedef struct {
  // Index of the first unparsed token.
  size_t first_unparsed_token;
  // The kinds that would have been accepted for the next token.
  TokenKind expected[MAX_NUM_EXPECTED];
  size_t num_expected;
} ErrorData;

void error_data_add(ErrorData *data, size_t token_index,
                    TokenKind expected_kind) {
  if (data->first_unparsed_token > token_index)
    return;
  if (data->first_unparsed_token < token_index)
    data->num_expected = 0;

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

#define GET(index) node_array->data[index]

#define YIELD(KIND, $kind, ...)                                                \
  yield(node_array, node_index,                                                \
        (ASTNode){.span = span,                                                \
                  .tree_size = tree_size,                                      \
                  .kind = KIND,                                                \
                  .$kind = __VA_ARGS__});                                      \
  return true;

#define LOG(message, name) eprintf("%s %s\n", message, name)

#define OK                                                                     \
  {                                                                            \
    LOG("Exit OK:", parser_name);                                              \
    return true;                                                               \
  }
#define FAIL                                                                   \
  {                                                                            \
    *token_index = start_token_index;                                          \
    *node_index = start_node_index;                                            \
    LOG("Exit FAIL:", parser_name);                                            \
    return false;                                                              \
  }

#define PARSER_PROTOTYPE(name)                                                 \
  bool parse_##name(TokenStream stream, size_t *token_index,                   \
                    ASTNodeArray *node_array, size_t *node_index,              \
                    ErrorData *error_data)

#define BEGIN(name)                                                            \
  size_t start_token_index = *token_index;                                     \
  size_t start_node_index = *node_index;                                       \
  Token token;                                                                 \
  const char *parser_name = #name;                                             \
  LOG("Enter:", parser_name);                                                  \
  if (!peek_token(stream, *token_index, &token))                               \
    FAIL;                                                                      \
  Span span = (Span){.offset = token.offset, .length = 0};                     \
  size_t tree_size = 1;

#define PARSER(name, ...)                                                      \
  PARSER_PROTOTYPE(name) {                                                     \
    BEGIN(name);                                                               \
    __VA_ARGS__;                                                               \
  }

#define EXTEND_SPAN($span)                                                     \
  span.length = ($span).offset + ($span).length - span.offset

#define JOIN_SPANS(left, right) join_spans(GET(left).span, GET(right).span)

#define EXPECT(...)                                                            \
  if (!peek_token(stream, *token_index, &token))                               \
    FAIL;                                                                      \
  if (!IS_ONE_OF(token.kind, __VA_ARGS__)) {                                   \
    ERROR_DATA_ADD(__VA_ARGS__);                                               \
    FAIL;                                                                      \
  }                                                                            \
  *token_index += 1;                                                           \
  EXTEND_SPAN(token);

#define EXPECT_OP(...) EXPECT(__VA_ARGS__) TokenKind op = token.kind;

#define PARSE(name)                                                            \
  parse_##name(stream, token_index, node_array, node_index, error_data)

#define MUST_PARSE(name, out)                                                  \
  if (!PARSE(name))                                                            \
    FAIL;                                                                      \
  size_t out = *node_index;                                                    \
  tree_size += GET(out).tree_size;

#define TRY_PARSE(name)                                                        \
  if (PARSE(name))                                                             \
    OK;

#define ZERO_OR_MORE(element, nodes)                                           \
  size_t nodes = *node_index;                                                  \
  size_t num_##nodes = 0;                                                      \
  while (PARSE(element)) {                                                     \
    num_##nodes++;                                                             \
    tree_size += GET(*node_index - 1).tree_size;                               \
  }                                                                            \
  if (num_##nodes > 0)                                                         \
    EXTEND_SPAN(GET(*node_index - 1).span);

PARSER(name, {
  EXPECT(tIDENT);
  YIELD(NAME, name, {token.text, token.length});
});

PARSER(integer, {
  EXPECT(tNUMBER);
  YIELD(INTEGER, integer, {token.text, token.length});
});

PARSER(string, {
  EXPECT(tSTRING);
  YIELD(STRING, string, {token.text + 1, token.length - 2});
});

PARSER_PROTOTYPE(expr);
PARSER_PROTOTYPE(stmt);

PARSER(unary, {
  EXPECT_OP(tMINUS);
  MUST_PARSE(expr, expr);
  YIELD(UNARY, unary, {.op = op, .expr = expr});
});

PARSER(binary, {
  MUST_PARSE(expr, left);
  EXPECT_OP(tPLUS, tMINUS, tSTAR, tSLASH);
  MUST_PARSE(expr, right);
  YIELD(BINARY, binary, {.op = op, .left = left, .right = right});
});

PARSER(expr, {
  TRY_PARSE(name);
  TRY_PARSE(integer);
  TRY_PARSE(string);
  TRY_PARSE(unary);
  TRY_PARSE(binary);
  FAIL;
});

PARSER_PROTOTYPE(declaration);

PARSER(assign, {
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ);
  MUST_PARSE(expr, value);
  YIELD(ASSIGN, assign, {.op = op, .name = name, .value = value});
});

bool parse_block(TokenStream stream, size_t *token_index,
                 ASTNodeArray *node_array, size_t *node_index,
                 ErrorData *error_data, size_t *num_out, Span *out_span,
                 size_t *out_tree_size) {
  BEGIN(block);
  EXPECT(tLBRACE);

  ZERO_OR_MORE(stmt, stmts)
  *num_out = num_stmts;

  EXPECT(tRBRACE)
  *out_span = span;
  *out_tree_size += tree_size;
  OK;
}

#define MUST_PARSE_BLOCK                                                       \
  size_t stmts = *node_index;                                                  \
  size_t num_stmts = 0;                                                        \
  Span block_span;                                                             \
  if (!parse_block(stream, token_index, node_array, node_index, error_data,    \
                   &num_stmts, &block_span, &tree_size))                       \
    FAIL;                                                                      \
  EXTEND_SPAN(block_span)

PARSER(for_loop, {
  MUST_PARSE(declaration, init);
  MUST_PARSE(expr, cond);
  MUST_PARSE(assign, step);
  MUST_PARSE_BLOCK;

  YIELD(FOR_LOOP, for_loop,
        {.init = init,
         .cond = cond,
         .step = step,
         .stmts = stmts,
         .num_stmts = num_stmts});
});

PARSER(declaration, {
  MUST_PARSE(name, name)
  EXPECT(tCOLONEQ)
  MUST_PARSE(expr, value)

  YIELD(DECLARATION, declaration, {.name = name, .value = value});
});

PARSER(stmt, {
  // FIXME: no semicolons
  TRY_PARSE(expr);
  TRY_PARSE(for_loop);
  TRY_PARSE(assign);
  TRY_PARSE(declaration);
  FAIL;
});

PARSER(param, {
  MUST_PARSE(name, name)
  MUST_PARSE(name, type)

  YIELD(PARAM, param, {.name = name, .type = type});
});

PARSER(function, {
  EXPECT(tFUNC)
  MUST_PARSE(name, name)
  ZERO_OR_MORE(param, params)
  MUST_PARSE(expr, returnType)
  MUST_PARSE_BLOCK;

  YIELD(FUNCTION, function,
        {.name = name,
         .params = params,
         .num_params = num_params,
         .stmts = stmts,
         .num_stmts = num_stmts});
});

PARSER(def, {
  TRY_PARSE(function);
  TRY_PARSE(declaration);
  FAIL;
});

PARSER(module, {
  ZERO_OR_MORE(def, defs);
  YIELD(MODULE, module, {defs, num_defs});
});

void print_parse_error(TokenStream stream, ErrorData error_data) {
  Token token = stream.data[error_data.first_unparsed_token];
  eprintf(
      "Parse error: unexpected token `%.*s` at offset %zu. Expected one of [",
      (int)token.length, token.text, token.offset);
  for (size_t i = 0; i < error_data.num_expected; i++) {
    TokenKind expected = error_data.expected[i];
    if (i > 0)
      eprintf(", ");
    eprintf("`%s`", lexer_token_display_name(expected));
  }
  eprintf("].\n");
}

bool parse(TokenStream stream, ASTNode **out) {
  size_t token_index = 0;
  ASTNodeArray node_array = {.data = NULL, .length = 0};
  size_t node_index = 0;
  ErrorData error_data = {0};
  bool result =
      parse_module(stream, &token_index, &node_array, &node_index, &error_data);
  if (token_index < stream.length) {
    print_parse_error(stream, error_data);
    return false;
  }
  *out = node_array.data;
  return result;
}
