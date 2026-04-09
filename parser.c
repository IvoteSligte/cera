#include "parser.h"
#include "ast.h"

// FIXME: spaces not being skipped

#define PARSE(name, out) parse_##name(tokens, num_tokens, offset, out)

#define MUST_PARSE(name, out)                                                  \
  if (!PARSE(name, out))                                                       \
    return false;

#define TRY_PARSE(name, out)                                                   \
  if (PARSE(name, out))                                                        \
    return true;

#define MUST_PARSE_NEW(name, decl_name)                                        \
  ASTNode *decl_name = calloc(sizeof(ASTNode), 1);                             \
  MUST_PARSE(name, decl_name)

#define ZERO_OR_MORE(element, nodes, $span)                                    \
  ASTNode *nodes = NULL;                                                       \
  size_t num_##nodes = 0;                                                      \
  Span $span;                                                                  \
  {                                                                            \
    ASTNode node = {0};                                                        \
                                                                               \
    while (PARSE(element, &node)) {                                            \
      ASTNode *new_##nodes = calloc(sizeof(ASTNode), num_##nodes + 1);         \
      memcpy(new_##nodes, nodes, num_##nodes);                                 \
      nodes[num_##nodes] = node;                                               \
      num_##nodes++;                                                           \
    }                                                                          \
    $span = num_##nodes == 0                                                   \
                ? (Span){0}                                                    \
                : join_spans(nodes[0].span, nodes[num_##nodes - 1].span);      \
  }

#define DEF_TOKEN                                                              \
  if (*offset >= num_tokens) {                                                 \
    return false;                                                              \
  }                                                                            \
  Token token = tokens[*offset];

#define CHECK_KIND(...)                                                        \
  if (!IS_ONE_OF(token.kind, __VA_ARGS__)) {                                   \
    return false;                                                              \
  }

#define DEF_CHECK(...)                                                         \
  DEF_TOKEN                                                                    \
  CHECK_KIND(__VA_ARGS__)                                                      \
  *offset += 1;

PARSER(_name) {
  DEF_CHECK(tIDENT)
  *out = (ASTNode){.span = token_span(token),
                   .kind = NAME,
                   .name = {token.text, token.length}};
  return true;
}

PARSER(_integer) {
  DEF_CHECK(tNUMBER)
  *out = (ASTNode){.span = token_span(token),
                   .kind = INTEGER,
                   .integer = {token.text, token.length}};
  return true;
}

PARSER(_string) {
  DEF_CHECK(tSTRING)
  *out = (ASTNode){.span = token_span(token),
                   .kind = STRING,
                   .integer = {token.text + 1, token.length - 2}};
  return true;
}

PARSER(_expr);
PARSER(_stmt);

PARSER(_unary) {
  Span span;

  DEF_CHECK(tMINUS)
  TokenKind op = token.kind;
  span.offset = token.offset;

  MUST_PARSE_NEW(expr, child)

  *out = (ASTNode){
      .span = token_span(token),
      .kind = UNARY,
      .unary = {op, child},
  };
  return true;
}

PARSER(_binary) {
  MUST_PARSE_NEW(expr, left)

  DEF_CHECK(tPLUS, tMINUS, tSTAR, tSLASH)
  TokenKind op = token.kind;

  MUST_PARSE_NEW(expr, right)

  *out = (ASTNode){
      .span = join_spans(left->span, right->span),
      .kind = BINARY,
      .binary = {op, left, right},
  };
  return true;
}

PARSER(_expr) {
  TRY_PARSE(name, out)
  TRY_PARSE(integer, out)
  TRY_PARSE(string, out)
  TRY_PARSE(unary, out)
  TRY_PARSE(binary, out)
  return false;
}

PARSER(_declaration);

PARSER(_assign) {
  MUST_PARSE_NEW(name, name)

  DEF_CHECK(tEQ)
  TokenKind op = token.kind;

  MUST_PARSE_NEW(expr, value)

  *out = (ASTNode){
      .span = join_spans(name->span, value->span),
      .kind = ASSIGN,
      .assign = {.op = op, .name = name, .value = value},
  };
  return true;
}

bool parse_block(Token *tokens, size_t num_tokens, size_t *offset,
                 ASTNode **out, size_t *num_out, Span *span) {
  {
    DEF_TOKEN
    CHECK_KIND(tLBRACE)
    span->offset = token.offset;
    *offset += 1;
  }

  {
    ZERO_OR_MORE(stmt, stmts, _span)
    *out = stmts;
    *num_out = num_stmts;
  }

  {
    DEF_TOKEN
    CHECK_KIND(tRBRACE)
    span->length = token.length;
    *offset += 1;
  }

  return true;
}

#define MUST_PARSE_BLOCK                                                       \
  ASTNode *stmts;                                                              \
  size_t num_stmts;                                                            \
  Span block_span;                                                             \
                                                                               \
  if (!parse_block(tokens, num_tokens, offset, &stmts, &num_stmts,             \
                   &block_span)) {                                             \
    return false;                                                              \
  }

PARSER(_for_loop) {
  MUST_PARSE_NEW(declaration, init)
  MUST_PARSE_NEW(expr, cond)
  MUST_PARSE_NEW(assign, step)

  MUST_PARSE_BLOCK

  *out = (ASTNode){
      .span = join_spans(init->span, block_span),
      .kind = FOR_LOOP,
      .for_loop = {.init = init,
                   .cond = cond,
                   .step = step,
                   .stmts = stmts,
                   .num_stmts = num_stmts},
  };
  return true;
}

PARSER(_declaration) {
  MUST_PARSE_NEW(name, name)

  DEF_CHECK(tCOLONEQ)
  TokenKind op = token.kind;

  MUST_PARSE_NEW(expr, value)

  *out = (ASTNode){
      .span = join_spans(name->span, value->span),
      .kind = DECLARATION,
      .declaration = {.name = name, .value = value},
  };
  return true;
}

PARSER(_stmt) {
  // FIXME: no semicolons
  TRY_PARSE(expr, out)
  TRY_PARSE(for_loop, out)
  TRY_PARSE(assign, out)
  TRY_PARSE(declaration, out)
  return false;
}

PARSER(_param) {
  MUST_PARSE_NEW(name, name)
  MUST_PARSE_NEW(name, type)

  *out = (ASTNode){
      .span = join_spans(name->span, type->span),
      .kind = PARAM,
      .param = {.name = name, .type = type},
  };
  return true;
}

PARSER(_function) {
  Span span;
  DEF_CHECK(tFUNC)
  span.offset = token.offset;

  MUST_PARSE_NEW(name, name)
  ZERO_OR_MORE(param, params, _span)

  MUST_PARSE_NEW(expr, returnType)

  MUST_PARSE_BLOCK

  *out = (ASTNode){.span = join_spans(span, block_span),
                   .kind = FUNCTION,
                   .function = {.name = name,
                                .params = params,
                                .num_params = num_params,
                                .stmts = stmts,
                                .num_stmts = num_stmts}};
  return false;
}

PARSER(_def) {
  TRY_PARSE(function, out)
  TRY_PARSE(declaration, out)
  return false;
}

PARSER() {
  ZERO_OR_MORE(def, defs, span)
  *out = (ASTNode){.span = span, .kind = MODULE, .module = {defs, num_defs}};
  return true;
}
