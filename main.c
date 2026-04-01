#include "lib/basic.h"

void parse(String source) {
  size_t line = 1;
  size_t column = 0;
  size_t indent = 0;

  size_t i = 0;
  while (i < source.len) {
    char c = str_at(source, i);
    // whitespace
    if (is_space(c)) {
      i++;
      continue;
    }
    // EOF
    if (c == EOF)
      return;
    // indentation
    if (c == '\n') {
    newline:
      line++;
      column = 0;
      i++;
      size_t indent = 0;
      char c = str_at(source, i);
      switch (c) {
      case ' ':
        column++;
        i++;
        break;
      case '\t':
        column = ((column / 4) + 1) * 4; // round to next multiple of 4
        i++;
        break;
      case '\n':
        goto newline;
      }
      indent = column;
      continue;
    }
    // identifier
    if (is_alpha(c)) {
      column++;
      String ident = {0};
      do {
        str_push(&ident, c);
        i++;
        char c = str_at(source, i);
      } while (is_alnum(c));
      continue;
    }
    // string literal
    if (c == '"') {
      i++;
      char c = str_at(source, i);
      
    }
  }
}

int main() { String source = read_file_to_string("test.xl"); }
