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
         memcmp(left.text, right.text, left.length) == 0;
}

bool name_eq_string(Name name, const char *string) {
  return name_eq(name, (Name){.text = string, .length = strlen(string)});
}

bool type_eq(Type left, Type right) {
  if (left.kind != right.kind) {
    return false;
  }
  if (left.kind == tyFUNCTION) {
    if (left.function.params.length != right.function.params.length)
      return false;
    for (size_t i = 0; i < left.function.params.length; i++) {
      if (!type_eq(left.function.params.data[i], right.function.params.data[i]))
        return false;
    }
    bool left_has_return = left.function._return != NULL;
    bool right_has_return = left.function._return != NULL;
    if (!left_has_return || !right_has_return)
      return !left_has_return && !right_has_return;
    else
      return type_eq(*left.function._return, *right.function._return);
  }
  if (left.kind == tySTRUCT) {
    return left._struct == right._struct;
  }
  if (left.kind == tyUNION) {
    panicf("unimplemented: type_eq for tyUNION");
  }
  return true;
}

#define N(name)                                                                \
  case (ty##name):                                                             \
    return #name;

const char *type_name(TypeKind kind) {
  switch (kind) {
    N(UNKNOWN);
    N(VOID);
    N(INT);
    N(BOOL);
    N(STRING);
    N(FUNCTION);
    N(STRUCT);
    N(UNION);
    N(TYPE);
  }
  panicf("Unknown type kind: %d", kind);
}
#undef N

void free_ast(AST *ast) {
  la_free_all(&ast->list_allocator);
  ra_free_all(&ast->random_allocator);
  ast->head = NULL;
}

#define VISIT($node) ast_visit($node, depth + 1, callback_data, callback)

#define VISIT_ARRAY(array) ITER_ARRAY(array, element, { VISIT(element); });

#define PCASE($name, $action)                                                  \
  CASE($name, {                                                                \
    $action;                                                                   \
    return;                                                                    \
  })

void ast_visit(ASTNode *node, size_t depth, void *callback_data,
               void(callback)(ASTNode *node, size_t depth, void *data)) {
  if (node == NULL)
    return;
  callback(node, depth, callback_data);

  SWITCH(node, {
    PCASE(name, {});
    PCASE(integer, {});
    PCASE(boolean, {});
    PCASE(string, {});
    PCASE(unary, { VISIT(unary->expr); });
    PCASE(binary, {
      VISIT(binary->left);
      VISIT(binary->right);
    });
    PCASE(function_call, {
      VISIT(function_call->function);
      VISIT_ARRAY(function_call->args);
    });
    PCASE(function, {
      VISIT_ARRAY(function->params);
      VISIT(function->return_type);
      VISIT_ARRAY(function->stmts);
    });
    PCASE(param, {
      VISIT(param->name);
      VISIT(param->type);
    });
    PCASE(if_stmt, {
      VISIT(if_stmt->cond);
      VISIT_ARRAY(if_stmt->then_stmts);
      VISIT_ARRAY(if_stmt->else_stmts);
    });
    PCASE(while_loop, {
      VISIT(while_loop->cond);
      VISIT_ARRAY(while_loop->stmts);
    });
    PCASE(for_loop, {
      VISIT(for_loop->init);
      VISIT(for_loop->cond);
      VISIT(for_loop->step);
      VISIT_ARRAY(for_loop->stmts);
    });
    PCASE(assign, {
      VISIT(assign->target);
      VISIT(assign->expr);
    });
    PCASE(return_stmt, { VISIT(return_stmt->expr); });
    PCASE(field, {
      VISIT(field->name);
      VISIT(field->type);
    });
    PCASE(_struct, { VISIT_ARRAY(_struct->fields); });
    PCASE(decl, {
      VISIT(decl->name);
      VISIT(decl->expr);
    })
    PCASE(module, { VISIT_ARRAY(module->decls); });
  });
  panicf("visit not implemented for node: %s", ast_node_name(node->kind));
}

static void print_node(ASTNode *node, size_t depth, void *data) {
  UNUSED(data);
  printf("%*.*s", (int)depth, (int)depth, " ");
  SWITCH(node, {
    PCASE(name,
          printf("name: %.*s\n", (int)name->name.length, name->name.text));
    PCASE(integer,
          printf("integer: %.*s\n", (int)integer->length, integer->text));
    PCASE(boolean,
          printf("boolean: %.*s\n", (int)boolean->length, boolean->text));
    PCASE(string,
          printf("string: \"%.*s\"\n", (int)string->length, string->text));
    PCASE(unary, printf("unary: `%s`\n", token_name(unary->op)));
    PCASE(binary, printf("binary: `%s`\n", token_name(binary->op)));
    PCASE(function_call, printf("function_call:\n"));
    PCASE(function, printf("function:\n"));
    PCASE(param, printf("param:\n"));
    PCASE(if_stmt, printf("if_stmt:\n"));
    PCASE(while_loop, printf("while_loop:\n"));
    PCASE(for_loop, printf("for_loop:\n"));
    PCASE(assign, printf("assign: `%s`\n", token_name(assign->op)));
    PCASE(return_stmt, printf("return_stmt:\n"));
    PCASE(field, printf("field:\n"));
    PCASE(_struct, printf("struct:\n"));
    PCASE(decl, printf("decl:\n"));
    PCASE(module, printf("module:\n"));
  });
  panicf("print not implemented for node: %s", ast_node_name(node->kind));
}

void ast_print_nodes(ASTNode *node) { ast_visit(node, 0, NULL, print_node); }

#define N(name)                                                                \
  case (a##name):                                                              \
    return #name;

const char *ast_node_name(ASTNodeKind kind) {
  switch (kind) {
    N(INVALID);
    N(NAME);
    N(INTEGER);
    N(BOOLEAN);
    N(STRING);
    N(UNARY);
    N(BINARY);
    N(FUNCTION_CALL);
    N(FUNCTION);
    N(PARAM);
    N(IF_STMT);
    N(WHILE_LOOP);
    N(FOR_LOOP);
    N(ASSIGN);
    N(RETURN_STMT);
    N(FIELD);
    N(STRUCT);
    N(DECL);
    N(MODULE);
  }
  panicf("Unknown node kind: %d", kind)
}
#undef N

StructID add_struct(RandomAllocator *allocator, StructList *list) {
  StructID id = list->length;
  list->data = ra_recalloc(allocator, list->data, list->length + 1);
  list->length++;
  return id;
}

bool get_field_type(StructInfo *_struct, Name name, Type *out_type) {
  for (size_t i = 0; i < _struct->fields.length; i++) {
    FieldInfo field = _struct->fields.data[i];
    if (name_eq(field.name, name)) {
      *out_type = field.type;
      return true;
    }
  }
  return false;
}
