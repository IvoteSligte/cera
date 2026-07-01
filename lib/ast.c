#include "ast.h"
#include "ast_macro.h"

#include <stdalign.h>

Span token_span(Token token) {
  return (Span){.offset = token.offset, .length = token.length};
}

Span join_spans(Span left, Span right) {
  return (Span){
      .offset = left.offset,
      .length = right.offset - left.offset + right.length,
  };
}

char *name_dup_to_string(Name name) { return strndup(name.text, name.length); }

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
    // signed integers
    N(I8);
    N(I16);
    N(I32);
    N(I64);
    N(INT);
    // unsigned integers
    N(BOOL);
    N(U8);
    N(U16);
    N(U32);
    N(U64);
    N(UINT);
    // other
    N(STR);
    N(CHAR);
    N(PTR);
    N(FUNCTION);
    N(STRUCT);
    N(UNION);
    N(ARRAY);
    N(PRIM_TYPE);
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
    PCASE(character, {});
    PCASE(unary, { VISIT(unary->expr); });
    PCASE(binary, {
      assert(binary->left != node);
      assert(binary->right != node);
      VISIT(binary->left);
      VISIT(binary->right);
    });
    PCASE(func_call, {
      VISIT(func_call->function);
      VISIT_ARRAY(func_call->args);
    });
    PCASE(ptr_create, { VISIT(ptr_create->expr); });
    PCASE(ptr_deref, { VISIT(ptr_deref->expr); });
    PCASE(ptr_type, { VISIT(ptr_type->expr); });
    PCASE(index_op, {
      VISIT(index_op->expr);
      VISIT(index_op->index);
    });
    PCASE(func_decl, {
      VISIT(func_decl->name);
      VISIT_ARRAY(func_decl->params);
      VISIT(func_decl->return_type);
      VISIT_ARRAY(func_decl->stmts);
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
    PCASE(break_stmt, {});
    PCASE(continue_stmt, {});
    PCASE(field, {
      VISIT(field->name);
      VISIT(field->type);
    });
    PCASE(struct_decl, {
      VISIT(struct_decl->name);
      VISIT_ARRAY(struct_decl->fields);
    });
    PCASE(field_inst, {
      VISIT(field_inst->name);
      VISIT(field_inst->expr);
    });
    PCASE(struct_inst, {
      VISIT(struct_inst->type);
      VISIT_ARRAY(struct_inst->fields);
    });
    PCASE(member, {
      VISIT(member->expr);
      VISIT(member->name);
    });
    PCASE(var_decl, {
      VISIT(var_decl->name);
      VISIT(var_decl->expr);
    })
    PCASE(module, { VISIT_ARRAY(module->decls); });
  });
  panicf("visit not implemented for node: %s", ast_node_name(node->kind));
}

// TODO: output stream as parameter
static void print_node(ASTNode *node, size_t depth, void *data) {
  UNUSED(data);
  eprintf("%*.*s", (int)depth, (int)depth, " ");
  SWITCH(node, {
    PCASE(name,
          eprintf("name: %.*s\n", (int)name->name.length, name->name.text));
    PCASE(integer,
          eprintf("integer: %.*s\n", (int)integer->length, integer->text));
    PCASE(boolean,
          eprintf("boolean: %.*s\n", (int)boolean->length, boolean->text));
    PCASE(string,
          eprintf("string: \"%.*s\"\n", (int)string->length, string->text));
    PCASE(character, eprintf("character: '%.*s'\n", (int)character->length,
                             character->text));
    PCASE(unary, eprintf("unary: `%s`\n", token_name(unary->op)));
    PCASE(binary, eprintf("binary: `%s`\n", token_name(binary->op)));
    PCASE(func_call, eprintf("func_call:\n"));
    PCASE(ptr_create, eprintf("ptr_create:\n"));
    PCASE(ptr_deref, eprintf("ptr_deref:\n"));
    PCASE(ptr_type, eprintf("ptr_type:\n"));
    PCASE(index_op, eprintf("index_op:\n"));
    PCASE(func_decl, eprintf("func_decl:\n"));
    PCASE(param, eprintf("param:\n"));
    PCASE(if_stmt, eprintf("if_stmt:\n"));
    PCASE(while_loop, eprintf("while_loop:\n"));
    PCASE(for_loop, eprintf("for_loop:\n"));
    PCASE(assign, eprintf("assign: `%s`\n", token_name(assign->op)));
    PCASE(return_stmt, eprintf("return_stmt:\n"));
    PCASE(break_stmt, eprintf("break_stmt\n"));
    PCASE(continue_stmt, eprintf("continue_stmt\n"));
    PCASE(field, eprintf("field:\n"));
    PCASE(struct_decl, eprintf("struct_decl:\n"));
    PCASE(field_inst, eprintf("field_inst:\n"));
    PCASE(struct_inst, eprintf("struct_inst:\n"));
    PCASE(member, eprintf("member:\n"));
    PCASE(var_decl, eprintf("var_decl:\n"));
    PCASE(module, eprintf("module:\n"));
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
    N(CHARACTER);
    N(UNARY);
    N(BINARY);
    N(FUNC_CALL);
    N(PTR_CREATE);
    N(PTR_DEREF);
    N(PTR_TYPE);
    N(INDEX_OP);
    N(FUNC_DECL);
    N(PARAM);
    N(IF_STMT);
    N(WHILE_LOOP);
    N(FOR_LOOP);
    N(ASSIGN);
    N(RETURN_STMT);
    N(BREAK_STMT);
    N(CONTINUE_STMT);
    N(FIELD);
    N(STRUCT_DECL);
    N(FIELD_INST);
    N(STRUCT_INST);
    N(MEMBER);
    N(VAR_DECL);
    N(MODULE);
  }
  panicf("Unknown node kind: %d", kind)
}
#undef N

bool is_integer(TypeKind type) {
  return IS_ONE_OF(type, tyI8, tyI16, tyI32, tyI64, tyINT, tyU8, tyU16, tyU32,
                   tyU64, tyUINT);
}

bool is_numeric(TypeKind type) { return is_integer(type); }

bool is_comparable(TypeKind type) {
  return !IS_ONE_OF(type, tySTRUCT, tyFUNCTION, tyUNION, tyPRIM_TYPE, tyVOID);
}

bool is_signed(TypeKind type) {
  return IS_ONE_OF(type, tyINT, tyI8, tyI16, tyI32, tyI64);
}
