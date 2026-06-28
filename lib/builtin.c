// file containing functions accessible to a Ceam program

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char *text;
  int64_t length;
} str;

// Converts a UTF-32 codepoint (uint32_t) to UTF-8 characters (char).
static size_t encode_utf8(uint32_t c, char out[4]) {
  if (c <= 0x7F) { // ascii
    out[0] = c;
    return 1;
  }
  if (c <= 0x7FF) {
    out[0] = 0xC0 | (c >> 6);
    out[1] = 0x80 | (c & 0x3F);
    return 2;
  }
  if (c <= 0xFFFF) {
    out[0] = 0xE0 | (c >> 12);
    out[1] = 0x80 | ((c >> 6) & 0x3F);
    out[2] = 0x80 | (c & 0x3F);
    return 3;
  }
  if (c <= 0x10FFFF) {
    out[0] = 0xF0 | (c >> 18);
    out[1] = 0x80 | ((c >> 12) & 0x3F);
    out[2] = 0x80 | ((c >> 6) & 0x3F);
    out[3] = 0x80 | (c & 0x3F);
    return 4;
  }
  return 0; // invalid codepoint
}

void print_bool(bool b) { printf("%s", b ? "true" : "false"); }

void print_int(int64_t n) { printf("%ld", n); }

void print_str(str string) {
  printf("%.*s", (int)string.length, string.text);
}

void print_char(uint32_t c) {
  char utf8[4] = {0};
  size_t length = encode_utf8(c, utf8);
  fwrite(utf8, 1, length, stdout);
}

void print_byte(uint8_t b) {
  fprintf(stderr, "byte: 0x%02x\n", b);
  fputc(b, stdout);
}

bool __str_eq(str left, str right) {
  return left.length == right.length &&
         (memcmp(left.text, right.text, left.length) == 0);
}
