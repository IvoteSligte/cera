#include "ast.h"
#include "ast_macro.h"

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

#define CASE(name, ...)                                                        \
  case UPPER_##name: {                                                         \
    __auto_type name = ast->name;                                              \
    __VA_ARGS__                                                                \
    break;                                                                     \
  }

static void visit_inner(ASTNode *node_array, size_t index,
                        void(callback)(ASTNode *)) {
  ASTNode *ast = &node_array[index];
  callback(ast);

  switch (ast->kind) {
    CASE(name, {});
    CASE(integer, {});
    CASE(string, {});
    CASE(unary, { VISIT(unary.expr); });
    CASE(binary, {
      VISIT(binary.left);
      VISIT(binary.right);
    });
    CASE(function, {
      VISIT(function.name);
      VISIT_ARRAY(function.params, function.num_params);
      VISIT(function.returnType);
      VISIT_ARRAY(function.stmts, function.num_stmts);
    });
    CASE(param, {
      VISIT(param.name);
      VISIT(param.type);
    });
    CASE(for_loop, {
      VISIT(for_loop.init);
      VISIT(for_loop.cond);
      VISIT(for_loop.step);
      VISIT_ARRAY(for_loop.stmts, for_loop.num_stmts);
    });
    CASE(assign, {
      VISIT(assign.name);
      VISIT(assign.value);
    });
    CASE(declaration, {
      VISIT(declaration.name);
      VISIT(declaration.value);
    })
    CASE(module, { VISIT_ARRAY(module.definitions, module.num_definitions); });
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
