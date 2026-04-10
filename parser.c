#include "parser.h"
#include "ast.h"

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

#define GET(index) node_array->data[index]

#define YIELD(...)                                                             \
  yield(node_array, node_index, __VA_ARGS__);                                  \
  return true;

#define PARSER(name)                                                           \
  bool parse_##name(TokenStream stream, size_t *token_index,                   \
                    ASTNodeArray *node_array, size_t *node_index)

#define BEGIN                                                                  \
  size_t start_token_index = *token_index;                                     \
  size_t start_node_index = *node_index;                                       \
  Token token;                                                                 \
  if (!peek_token(stream, *token_index, &token))                               \
    return false;                                                              \
  Span span = (Span){.offset = token.offset, .length = 0};                     \
  size_t tree_size = 1;

#define OK return true;
#define FAIL                                                                   \
  {                                                                            \
    *token_index = start_token_index;                                          \
    *node_index = start_node_index;                                            \
    return false;                                                              \
  }

#define EXTEND_SPAN($span)                                                     \
  span.length = ($span).offset + ($span).length - span.offset

#define JOIN_SPANS(left, right) join_spans(GET(left).span, GET(right).span)

#define EXPECT(...)                                                            \
  if (!peek_token(stream, *token_index, &token) ||                             \
      !IS_ONE_OF(token.kind, __VA_ARGS__))                                     \
    FAIL;                                                                      \
  *token_index += 1;                                                           \
  EXTEND_SPAN(token);

#define EXPECT_OP(...) EXPECT(__VA_ARGS__) TokenKind op = token.kind;

#define PARSE(name) parse_##name(stream, token_index, node_array, node_index)

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

PARSER(name) {
  BEGIN;
  EXPECT(tIDENT);
  YIELD((ASTNode){.span = span,
                  .tree_size = tree_size,
                  .kind = NAME,
                  .name = {token.text, token.length}});
}

PARSER(integer) {
  BEGIN;
  EXPECT(tNUMBER);
  YIELD((ASTNode){.span = span,
                  .tree_size = tree_size,
                  .kind = INTEGER,
                  .integer = {token.text, token.length}});
}

PARSER(string) {
  BEGIN;
  EXPECT(tSTRING);
  YIELD((ASTNode){.span = span,
                  .tree_size = tree_size,
                  .kind = STRING,
                  .integer = {token.text + 1, token.length - 2}});
}

PARSER(expr);
PARSER(stmt);

PARSER(unary) {
  BEGIN;
  EXPECT_OP(tMINUS);
  MUST_PARSE(expr, expr);
  YIELD((ASTNode){
      .span = span,
      .tree_size = tree_size,
      .kind = UNARY,
      .unary = {.op = op, .expr = expr},
  });
}

PARSER(binary) {
  BEGIN;
  MUST_PARSE(expr, left);
  EXPECT_OP(tPLUS, tMINUS, tSTAR, tSLASH);
  MUST_PARSE(expr, right);
  YIELD((ASTNode){.span = span,
                  .tree_size = tree_size,
                  .kind = BINARY,
                  .binary = {.op = op, .left = left, .right = right}});
}

PARSER(expr) {
  BEGIN;
  TRY_PARSE(name);
  TRY_PARSE(integer);
  TRY_PARSE(string);
  TRY_PARSE(unary);
  TRY_PARSE(binary);
  FAIL;
}

PARSER(declaration);

PARSER(assign) {
  BEGIN;
  MUST_PARSE(name, name);
  EXPECT_OP(tEQ);
  MUST_PARSE(expr, value);
  YIELD((ASTNode){
      .span = span,
      .tree_size = tree_size,
      .kind = ASSIGN,
      .assign = {.op = op, .name = name, .value = value},
  });
}

bool parse_block(TokenStream stream, size_t *token_index,
                 ASTNodeArray *node_array, size_t *node_index, size_t *num_out,
                 Span *out_span, size_t *out_tree_size) {
  BEGIN;
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
  if (!parse_block(stream, token_index, node_array, node_index, &num_stmts,    \
                   &block_span, &tree_size))                                   \
    FAIL;                                                                      \
  EXTEND_SPAN(block_span)

PARSER(for_loop) {
  BEGIN;
  MUST_PARSE(declaration, init);
  MUST_PARSE(expr, cond);
  MUST_PARSE(assign, step);
  MUST_PARSE_BLOCK;

  YIELD((ASTNode){
      .span = span,
      .tree_size = tree_size,
      .kind = FOR_LOOP,
      .for_loop = {.init = init,
                   .cond = cond,
                   .step = step,
                   .stmts = stmts,
                   .num_stmts = num_stmts},
  });
}

PARSER(declaration) {
  BEGIN
  MUST_PARSE(name, name)
  EXPECT(tCOLONEQ)
  MUST_PARSE(expr, value)

  YIELD((ASTNode){
      .span = span,
      .tree_size = tree_size,
      .kind = DECLARATION,
      .declaration = {.name = name, .value = value},
  });
}

PARSER(stmt) {
  BEGIN;
  // FIXME: no semicolons
  TRY_PARSE(expr);
  TRY_PARSE(for_loop);
  TRY_PARSE(assign);
  TRY_PARSE(declaration);
  FAIL;
}

PARSER(param) {
  BEGIN
  MUST_PARSE(name, name)
  MUST_PARSE(name, type)

  YIELD((ASTNode){
      .span = span,
      .tree_size = tree_size,
      .kind = PARAM,
      .param = {.name = name, .type = type},
  });
}

PARSER(function) {
  BEGIN
  EXPECT(tFUNC)
  MUST_PARSE(name, name)
  ZERO_OR_MORE(param, params)
  MUST_PARSE(expr, returnType)
  MUST_PARSE_BLOCK;

  YIELD((ASTNode){
      .span = span,
        .tree_size = tree_size,      
                  .kind = FUNCTION,
                  .function = {.name = name,
                               .params = params,
                               .num_params = num_params,
                               .stmts = stmts,
                               .num_stmts = num_stmts}});
}

PARSER(def) {
  BEGIN;
  TRY_PARSE(function);
  TRY_PARSE(declaration);
  FAIL;
}

PARSER(module) {
  BEGIN;
  ZERO_OR_MORE(def, defs);
  YIELD((ASTNode){.span = span, .tree_size = tree_size, .kind = MODULE, .module = {defs, num_defs}});
}

bool parse(TokenStream stream, ASTNode **out) {
  size_t token_index = 0;
  ASTNodeArray node_array = {.data = NULL, .length = 0};
  size_t node_index = 0;
  bool result = parse_module(stream, &token_index, &node_array, &node_index);
  *out = node_array.data;
  return result;
}
