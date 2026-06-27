#pragma once

#include "ast.h"
#include "error.h"

bool parse_and_analyze(const char *source, AST *out_ast,
                       CompileErrors *out_errors);
CompileErrors compile_and_run(const char *source);
CompileErrors compile_to_object_file(const char *source,
                                     const char *output_file);

// Parse and analyze the given code, but do not return the AST.
CompileErrors diagnose(const char *source);

// Links an object file with the standard library and libc, producing a binary.
bool link_to_binary(const char *object_file, const char *output_file);
