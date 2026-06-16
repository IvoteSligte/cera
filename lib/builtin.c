// file containing functions accessible to a Ceam program

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  char *text;
  int64_t length;
} CeamString;

void print_bool(bool b) { printf("%s", b ? "true" : "false"); }

void print_int(int64_t n) { printf("%ld", n); }

void print_string(CeamString string) {
  printf("%.*s", (int)string.length, string.text);
}
