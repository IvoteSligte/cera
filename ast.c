#include "ast.h"

Span token_span(Token token) {
  return (Span){.offset = token.offset, .length = token.length};
}

Span join_spans(Span left, Span right) {
  return (Span){
      .offset = left.offset,
      .length = right.offset - left.offset + right.length,
  };
}

void free_ast(ASTNode *node_array) { free(node_array); }

#define VISIT(index) visit_inner(node_array, index, callback)

#define VISIT_ARRAY($start, $length)                                           \
  {                                                                            \
    size_t index = $start;                                                     \
    for (size_t i = 0; i < ($length); i++) {                                   \
      VISIT(index);                                                            \
      index += node_array[index].tree_size;                                    \
    }                                                                          \
  }

static void visit_inner(ASTNode *node_array, size_t index,
                        void(callback)(ASTNode *)) {
  ASTNode *ast = &node_array[index];
  callback(ast);

  switch (ast->kind) {
  case NAME:
    return;
  case INTEGER:
    return;
  case STRING:
    return;
  case UNARY:
    VISIT(ast->unary.expr);
    return;
  case BINARY:
    VISIT(ast->binary.left);
    VISIT(ast->binary.right);
    return;
  case FUNCTION:
    VISIT(ast->function.name);
    VISIT_ARRAY(ast->function.params, ast->function.num_params);
    VISIT(ast->function.returnType);
    VISIT_ARRAY(ast->function.stmts, ast->function.num_stmts);
    return;
  case PARAM:
    VISIT(ast->param.name);
    VISIT(ast->param.type);
    return;
  case FOR_LOOP:
    VISIT(ast->for_loop.init);
    VISIT(ast->for_loop.cond);
    VISIT(ast->for_loop.step);
    VISIT_ARRAY(ast->for_loop.stmts, ast->for_loop.num_stmts);
    return;
  case ASSIGN:
    VISIT(ast->assign.name);
    VISIT(ast->assign.value);
    return;
  case DECLARATION:
    VISIT(ast->declaration.name);
    VISIT(ast->declaration.value);
    return;
  case MODULE:
    VISIT_ARRAY(ast->module.definitions, ast->module.num_definitions);
    return;
  }
}

void visit(ASTNode *node_array, void(callback)(ASTNode *)) {
  visit_inner(node_array, 0, callback);
}

static void print_node(ASTNode *node) {
  switch (node->kind) {
  case NAME:
    printf("name: `%.*s`\n", (int)node->name.length, node->name.text);
    break;
  case INTEGER:
    printf("integer: `%.*s`\n", (int)node->integer.length, node->integer.text);    
    break;
  case STRING:
    printf("string: `%.*s`\n", (int)node->string.length, node->string.text);
    break;
  case UNARY:
    printf("unary: `%s`\n", lexer_token_name(node->unary.op));
    break;
  case BINARY:
    printf("binary: `%s`\n", lexer_token_name(node->binary.op));
    break;
  case FUNCTION:
    printf("function:\n");
    break;
  case PARAM:
    printf("param:\n");
    break;
  case FOR_LOOP:
    printf("for_loop:\n");
    break;
  case ASSIGN:
    printf("assign: `%s`\n", lexer_token_name(node->assign.op));
    break;
  case DECLARATION:
    printf("declaration:\n");
    break;
  case MODULE:
    printf("module:\n");    
    break;
  }
}

void ast_print_nodes(ASTNode *node_array) { visit(node_array, print_node); }
