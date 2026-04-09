
#include "lexer.h"

int main() {
  const char *source = "void lexerDoStuff(int a, int b) { printf(\"hi %s\", \"name\"); }";
  size_t offset = 0;
  Token token;

  lexer_init();

  while (lex(source, &offset, &token)) {
    printf("%s `%.*s`\n", lexer_token_name(token.kind), (int)token.length, token.text);
  }
  if (offset != strlen(source)) {
    eprintf("Failed to match token in string: `%.*s`\n", 100, &source[offset]);
  }
  lexer_free();
}
