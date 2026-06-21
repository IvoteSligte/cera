// file containing functions accessible to a Ceam program

#include <stdio.h>
#include <string.h>

#include "builtin.h"

void print_bool(bool b) { printf("%s", b ? "true" : "false"); }

void print_int(int64_t n) { printf("%ld", n); }

void print_string(CeamString string) {
  printf("%.*s", (int)string.length, string.text);
}

bool __string_eq(CeamString left, CeamString right) {
  return left.length == right.length &&
         (memcmp(left.text, right.text, left.length) == 0);
}
