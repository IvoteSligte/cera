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


bool name_eq(Name left, Name right) {
  return strncmp(left.text, right.text, MIN(left.length, right.length));
}

void free_ast(ASTNode *node_array) { free(node_array); }

#define VISIT(index) ast_visit(node_array, index, depth + 1, callback)

#define VISIT_ARRAY($start, $length)                                           \
  {                                                                            \
    size_t index = $start;                                                     \
    for (size_t i = 0; i < ($length); i++) {                                   \
      VISIT(index);                                                            \
      index += node_array[index].tree_size;                                    \
    }                                                                          \
  }

void ast_visit(ASTNode *node_array, size_t index, size_t depth,
               void(callback)(ASTNode *node_array, size_t index,
                              size_t depth)) {
  ASTNode *node = &node_array[index];
  callback(node_array, index, depth);

  switch (node->kind) {
  case INVALID:
    panicf("Tried to visit invalid AST node.\n");
    break;

    CASE(name, {});
    CASE(integer, {});
    CASE(string, {});
    CASE(unary, { VISIT(unary.expr); });
    CASE(binary, {
      VISIT(binary.left);
      VISIT(binary.right);
    });
    CASE(function_call, {
      VISIT(function_call.name);
      VISIT_ARRAY(function_call.args, function_call.num_args);
    });
    CASE(function, {
      VISIT(function.name);
      VISIT_ARRAY(function.params, function.num_params);
      if (function.has_return_type)
        VISIT(function.return_type);
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

static void print_node(ASTNode *node_array, size_t index, size_t depth) {
  ASTNode *node = &node_array[index];
  printf("%*.*s", (int)depth, (int)depth, " ");
  switch (node->kind) {
  case INVALID:
    panicf("Tried to print invalid AST node.\n");
    break;

    CASE(name, {
      printf("name: `%.*s`\n", (int)node->name.length, node->name.text);
    });
    CASE(integer, {
      printf("integer: `%.*s`\n", (int)node->integer.length,
             node->integer.text);
    });
    CASE(string, {
      printf("string: `%.*s`\n", (int)node->string.length, node->string.text);
    });
    CASE(unary, { printf("unary: `%s`\n", token_name(node->unary.op)); });
    CASE(binary, { printf("binary: `%s`\n", token_name(node->binary.op)); });
    CASE(function_call, { printf("function_call:\n"); })
    CASE(function, { printf("function:\n"); });
    CASE(param, { printf("param:\n"); });
    CASE(for_loop, { printf("for_loop:\n"); });
    CASE(assign, { printf("assign: `%s`\n", token_name(node->assign.op)); });
    CASE(declaration, { printf("declaration:\n"); });
    CASE(module, { printf("module:\n"); });
  }
}

void ast_print_nodes(ASTNode *node_array, size_t index) {
  ast_visit(node_array, index, 0, print_node);
}

#define N(name)                                                                \
  case (name):                                                                 \
    return #name;

const char *ast_node_name(ASTNodeKind kind) {
  switch (kind) {
    N(INVALID);
    N(NAME);
    N(INTEGER);
    N(STRING);
    N(UNARY);
    N(BINARY);
    N(FUNCTION_CALL);
    N(FUNCTION);
    N(PARAM);
    N(FOR_LOOP);
    N(ASSIGN);
    N(DECLARATION);
    N(MODULE);
  }
  panicf("Unknown node kind: %d\n", kind)
}
