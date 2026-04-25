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
  return left.length == right.length &&
         strncmp(left.text, right.text, left.length);
}

bool type_eq(Type left, Type right) {
  if (left.kind != right.kind)
    return false;
  if (left.kind == tyFUNCTION) {
    if (left.function.num_params != right.function.num_params)
      return false;
    for (size_t i = 0; i < left.function.num_params; i++) {
      if (!type_eq(left.function.params[i], right.function.params[i]))
        return false;
    }
    return type_eq(*left.function._return, *right.function._return);
  }
  return true;
}

void free_ast(AST *ast) {
  la_free_all(&ast->allocator);
  ast->head = NULL;
}

#define VISIT(node) ast_visit(node, depth + 1, callback)

#define VISIT_ARRAY(array) ITER_ARRAY(array, element, { VISIT(element); });

void ast_visit(ASTNode *node, size_t depth,
               void(callback)(ASTNode *node, size_t depth)) {
  callback(node, depth);

  SWITCH(node, {
    CASE(name, {});
    CASE(integer, {});
    CASE(string, {});
    CASE(unary, { VISIT(unary->expr); });
    CASE(binary, {
      VISIT(binary->left);
      VISIT(binary->right);
    });
    CASE(function_call, {
      VISIT(function_call->function);
      VISIT_ARRAY(function_call->args);
    });
    CASE(function, {
      VISIT_ARRAY(function->params);
      if (function->return_type != NULL)
        VISIT(function->return_type);
      VISIT_ARRAY(function->stmts);
    });
    CASE(param, {
      VISIT(param->name);
      VISIT(param->type);
    });
    CASE(for_loop, {
      VISIT(for_loop->init);
      VISIT(for_loop->cond);
      VISIT(for_loop->step);
      VISIT_ARRAY(for_loop->stmts);
    });
    CASE(assign, {
      VISIT(assign->name);
      VISIT(assign->value);
    });
    CASE(return_stmt, { VISIT(return_stmt->expr); });
    CASE(declaration, {
      VISIT(declaration->name);
      VISIT(declaration->expr);
    })
    CASE(module, { VISIT_ARRAY(module->declarations); });
  });
}

static void print_node(ASTNode *node, size_t depth) {
  printf("%*.*s", (int)depth, (int)depth, " ");
  SWITCH(node, {
    CASE(name, {
      printf("name: `%.*s`\n", (int)name->name.length, name->name.text);
    });
    CASE(integer,
         { printf("integer: `%.*s`\n", (int)integer->length, integer->text); });
    CASE(string,
         { printf("string: `%.*s`\n", (int)string->length, string->text); });
    CASE(unary, { printf("unary: `%s`\n", token_name(unary->op)); });
    CASE(binary, { printf("binary: `%s`\n", token_name(binary->op)); });
    CASE(function_call, { printf("function_call:\n"); })
    CASE(function, { printf("function:\n"); });
    CASE(param, { printf("param:\n"); });
    CASE(for_loop, { printf("for_loop:\n"); });
    CASE(assign, { printf("assign: `%s`\n", token_name(assign->op)); });
    CASE(return_stmt, { printf("return_stmt:\n"); });
    CASE(declaration, { printf("declaration:\n"); });
    CASE(module, { printf("module:\n"); });
  });
}

void ast_print_nodes(ASTNode *node) { ast_visit(node, 0, print_node); }

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
    N(RETURN_STMT);
    N(DECLARATION);
    N(MODULE);
  }
  panicf("Unknown node kind: %d", kind)
}
