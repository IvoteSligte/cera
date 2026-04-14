#include "parser.h"
#include "ast.h"
#include "ast_macro.h"

#include <stdarg.h>

typedef struct {
  ASTNode *data;
  size_t length;
} ASTNodeArray;

void *zeroed_realloc(void *data, size_t current_nmemb, size_t new_nmemb,
                     size_t element_size) {
  assert(current_nmemb <= new_nmemb);
  data = realloc(data, new_nmemb * element_size);
  memset(data + (current_nmemb * element_size), 0,
         (new_nmemb - current_nmemb) * element_size);
  return data;
}

void yield(ASTNodeArray *node_array, size_t node_index, ASTNode node) {
  assert(node.tree_size > 0);
  if (node_index >= node_array->length) {
    size_t new_length = node_index + 1;
    node_array->data = zeroed_realloc(node_array->data, node_array->length,
                                      new_length, sizeof(ASTNode));
    node_array->length = new_length;
  }
  node_array->data[node_index] = node;
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

#define GET(index) node_array->data[index]

#define YIELD(KIND, $kind, ...)                                                \
  __RESERVED__ = true; /* checks that RESERVE has been used before YIELD */    \
  yield(node_array, start_node_index,                                          \
        (ASTNode){.span = span,                                                \
                  .tree_size = tree_size,                                      \
                  .kind = KIND,                                                \
                  .$kind = __VA_ARGS__})

#define RETURN($kind, ...)                                                     \
  YIELD(UPPER_##$kind, $kind, __VA_ARGS__);                                    \
  OK;

int log_indent = 0;

#define LOG(format, ...)                                                       \
  eprintf("%-4zu %-3zu %*.*s" format "\n", *token_index, *node_index,          \
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

#define OK                                                                     \
  {                                                                            \
    LOG_EXIT("OK");                                                            \
    return true;                                                               \
  }
#define FAIL                                                                   \
  {                                                                            \
    *token_index = start_token_index;                                          \
    *node_index = start_node_index;                                            \
    LOG_EXIT("FAIL");                                                          \
    return false;                                                              \
  }

#define PARSER_PROTOTYPE(name)                                                 \
  bool parse_##name(TokenStream stream, size_t *token_index,                   \
                    ASTNodeArray *node_array, size_t *node_index,              \
                    ParseError *error_data)

#define RESERVE                                                                \
  *node_index += 1;                                                            \
  bool __RESERVED__;

#define BEGIN(name)                                                            \
  const char *parser_name = #name;                                             \
  LOG_ENTER;                                                                   \
  size_t start_token_index = *token_index;                                     \
  size_t start_node_index = *node_index;                                       \
  Token token;                                                                 \
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

#define PARSE(name)                                                            \
  parse_##name(stream, token_index, node_array, node_index, error_data)

// Try to parse and return on failure.
#define MUST_PARSE(name, out)                                                  \
  size_t out = *node_index;                                                    \
  if (!PARSE(name))                                                            \
    FAIL;                                                                      \
  tree_size += GET(out).tree_size;

// Try to parse and continue on success.
// Declares has_##out indicating whether it succeeded.
#define MAY_PARSE(name, out)                                                   \
  size_t out = *node_index;                                                    \
  bool has_##out = PARSE(name);

// Try to parse and return on success.
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

#define ZERO_OR_MORE_SEPARATED(element, nodes, ...)                            \
  size_t nodes = *node_index;                                                  \
  size_t num_##nodes = 0;                                                      \
  while (PARSE(element)) {                                                     \
    num_##nodes++;                                                             \
    tree_size += GET(*node_index - 1).tree_size;                               \
    if (!peek_token(stream, *token_index, &token) ||                           \
        !IS_ONE_OF(token.kind, __VA_ARGS__)) {                                 \
      break;                                                                   \
    }                                                                          \
    *token_index += 1;                                                         \
    EXTEND_SPAN(token);                                                        \
  }                                                                            \
  if (num_##nodes > 0)                                                         \
    EXTEND_SPAN(GET(*node_index - 1).span);

PARSER_PROTOTYPE(expr);
PARSER_PROTOTYPE(stmt);

bool parse_block(TokenStream stream, size_t *token_index,
                 ASTNodeArray *node_array, size_t *node_index,
                 ParseError *error_data, size_t *num_out, Span *out_span,
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
  ImplicitArray stmts = {.start_index = *node_index, .length = 0};             \
  Span block_span;                                                             \
  if (!parse_block(stream, token_index, node_array, node_index, error_data,    \
                   &stmts.length, &block_span, &tree_size))                    \
    FAIL;                                                                      \
  EXTEND_SPAN(block_span);

PARSER(name, {
  RESERVE;
  EXPECT(tIDENT);
  RETURN(name, {token.text, token.length});
});

PARSER(integer, {
  RESERVE;
  EXPECT(tNUMBER);
  RETURN(integer, {token.text, token.length});
});

PARSER(string, {
  RESERVE;
  EXPECT(tSTRING);
  RETURN(string, {token.text + 1, token.length - 2});
});

PARSER(function_call, {
  RESERVE;
  MUST_PARSE(name, function);
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(expr, args, tCOMMA);
  EXPECT(tRPAREN);
  RETURN(function_call, {.function = function, .args = args});
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

  RESERVE;
  EXPECT_OP(tMINUS);
  MUST_PARSE(primary, expr);
  RETURN(unary, {.op = op, .expr = expr});
});

void fix_precedence(ASTNode *node_array, size_t index) {
  ASTNode *node = &node_array[index];
  assert(node->kind == BINARY);
  __auto_type binary = &node->binary;
  ASTNode *left_node = &node_array[binary->left];
  assert(left_node->kind != BINARY);
  ASTNode *right_node = &node_array[binary->right];

  if (right_node->kind == BINARY) {
    __auto_type right = &right_node->binary;

    if (!right->has_parens &&
        token_precedence(right->op) < token_precedence(binary->op)) {
      // performs a left-rotation on the tree
      // NOTE: this does not preserve order of nodes
      binary->right = right->left;
      right->left = index;
      SWAP(node, right_node);
    }
  }
}

PARSER(binary, {
  TRY_PARSE(unary);

  RESERVE;
  MUST_PARSE(unary, left);
  EXPECT_OP(tPLUS, tMINUS, tSTAR, tSLASH);
  MUST_PARSE(expr, right);

  YIELD(BINARY, binary,
        {.op = op, .has_parens = false, .left = left, .right = right});
  fix_precedence(node_array->data, *node_index - 1);
  OK;
});

PARSER(param, {
  RESERVE;
  MUST_PARSE(name, name);
  EXPECT(tCOL);
  MUST_PARSE(name, type);
  RETURN(param, {.name = name, .type = type});
});

PARSER(function, {
  RESERVE;
  EXPECT(tLPAREN);
  ZERO_OR_MORE_SEPARATED(param, params, tCOMMA);
  EXPECT(tRPAREN);
  MAY_PARSE(name, return_type);
  MUST_PARSE_BLOCK;
  RETURN(function, {.params = params,
                    .return_type = return_type,
                    .has_return_type = has_return_type,
                    .stmts = stmts});
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
  RESERVE;
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ);
  MUST_PARSE(expr, value);
  EXPECT(tSEMI);
  RETURN(assign, {.op = op, .name = name, .value = value});
});

PARSER(return_stmt, {
  RESERVE;
  EXPECT(tRETURN);
  MUST_PARSE(expr, expr);
  EXPECT(tSEMI);
  RETURN(return_stmt, {.expr = expr});
});

PARSER(for_loop, {
  RESERVE;
  MUST_PARSE(declaration, init);
  MUST_PARSE(expr, cond);
  MUST_PARSE(assign, step);
  MUST_PARSE_BLOCK;

  RETURN(for_loop, {.init = init, .cond = cond, .step = step, .stmts = stmts});
});

PARSER(declaration_value, {
  TRY_PARSE(expr);
  TRY_PARSE(function);
  OK;
});

PARSER(declaration, {
  RESERVE;
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
  RESERVE;
  ZERO_OR_MORE(declaration, declarations);
  RETURN(module, {declarations, num_declarations});
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
  Token token = stream.data[error_data.first_unparsed_token];
  eprintf("Parse error: unexpected token `%.*s` at offset %zu. "
          "Expected one of [",
          (int)token.length, token.text, token.offset);
  for (size_t i = 0; i < error_data.num_expected; i++) {
    TokenKind expected = error_data.expected[i];
    if (i > 0)
      eprintf(", ");
    eprintf("`%s`", token_display_name(expected));
  }
  eprintf("].\n");
  OffsetInfo oi = get_offset_info(source, token.offset);
  eprintf(">>> line %zu, column %d\n", oi.line_number, oi.column_number);
  eprintf(" | %.*s\n", oi.line_length, oi.line);
  eprintf(" | %*s", oi.column_number, " ");
  for (size_t i = 0; i < token.length; i++)
    eprintf("~");
  eprintf("\n");
}

bool parse(TokenStream stream, ASTNode **out, ParseError *error_data) {
  size_t token_index = 0;
  ASTNodeArray node_array = {.data = NULL, .length = 0};
  size_t node_index = 0;
  *error_data = (ParseError){0};
  bool result =
      parse_module(stream, &token_index, &node_array, &node_index, error_data);
  if (token_index < stream.length) {
    return false;
  }
  *out = node_array.data;
  return result;
}
