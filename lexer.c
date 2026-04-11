#include <regex.h> // TODO: cross-platform

#include "lexer.h"

typedef struct {
  TokenKind kind;
  const char *name;
  const char *display_name;
  const char *regex;
} Matcher;

#define M(name, display_name, regex) {t##name, #name, display_name, "^" regex}

const Matcher MATCHERS[] = {
    M(WHITESPACE, "whitespace", "[\n\t ]+"),
    M(COMMENT, "comment", "// .*\n"),
    // words
    M(IDENT, "identifier", "[a-zA-Z][a-zA-Z0-9]*"),
    M(NUMBER, "number", "[0-9]+"),
    M(STRING, "string", "\"[^\"]*\""),
    // symbols
    M(LPAREN, "(", "\\("),
    M(RPAREN, ")", "\\)"),
    M(LBRACE, "{", "\\{"),
    M(RBRACE, "}", "\\}"),
    M(LBRACKET, "[", "\\["),
    M(RBRACKET, "]", "]"),
    M(DOT, ".", "\\."),
    M(PLUS, "+", "\\+"),
    M(MINUS, "-", "\\-"),
    M(STAR, "*", "\\*"),
    M(SLASH, "/", "/"),
    M(HASHTAG, "#", "#"),
    M(SEMI, ";", ";"),
    M(COMMA, ",", ","),
    M(EQ, "=", "="),
    M(COLONEQ, ":=", ":="),
    // keywords
    M(STRUCT, "struct", "struct"),
    M(FUNC, "func", "func"),
    M(UNION, "union", "union"),
    M(ENUM, "enum", "enum"),
};
#undef M

#define NUM_MATCHERS (sizeof(MATCHERS) / sizeof(MATCHERS[0]))

regex_t regexes[NUM_MATCHERS];

void lexer_init(void) {
  for (size_t i = 0; i < NUM_MATCHERS; i++) {
    int result = regcomp(&regexes[i], MATCHERS[i].regex, REG_EXTENDED);
    if (result) {
      char errbuf[100];
      size_t err_size = regerror(result, &regexes[i], errbuf, 100);
      panicf("Failed to compile regex `%s`. Error: %s\n", MATCHERS[i].regex,
             errbuf);
    }
  }
}

void lexer_free(void) {
  for (size_t i = 0; i < NUM_MATCHERS; i++) {
    regfree(&regexes[i]);
  }
}

const char *lexer_token_name(TokenKind kind) { return MATCHERS[kind].name; }
const char *lexer_token_display_name(TokenKind kind) {
  return MATCHERS[kind].display_name;
}

LexResult lex(const char *source, size_t *offset, Token *out) {
  size_t longest_match = 0;

  if (*offset >= strlen(source)) {
    return LEX_EOF;
  }
  for (size_t i = 0; i < NUM_MATCHERS; i++) {
    regmatch_t match;
    int result = regexec(&regexes[i], &source[*offset], 1, &match, 0);
    if (result == REG_NOERROR) {
      *out = (Token){
          .offset = *offset,
          .length = match.rm_eo - match.rm_so, // end - start
          .text = &source[*offset],
          .kind = MATCHERS[i].kind,
      };
      longest_match = out->length;
      continue;
    }
    if (result == REG_NOMATCH) {
      continue;
    }
    char errbuf[100];
    size_t err_size = regerror(result, &regexes[i], errbuf, 100);
    panicf("Failed to run regex. Error: %s\n", errbuf);
  }
  *offset += longest_match;
  if (longest_match == 0) {
    return LEX_NO_MATCH;
  } else if (out->kind == tWHITESPACE || out->kind == tCOMMENT) {
    return lex(source, offset, out); // skip whitespace and comments
  } else {
    return LEX_OK;
  }
}

static void print_error(const char *source, size_t offset) {
  eprintf("Failed to match token in string: `%.*s`\n", 100, &source[offset]);
}

bool fill_token_stream(const char *source, TokenStream *out) {
  size_t offset = 0;
  Token token;
  LexResult result;
  *out = (TokenStream){0};

  while ((result = lex(source, &offset, &token)) == LEX_OK) {
    out->data = realloc(out->data, sizeof(Token) * (out->length + 1));
    out->data[out->length] = token;
    out->length++;
  }
  if (result != LEX_EOF) {
    print_error(source, offset);
    return false;
  }
  return true;
}

void free_token_stream(TokenStream stream) { free(stream.data); }

void lexer_print_tokens(const char *source) {
  size_t offset = 0;
  Token token;
  LexResult result;

  while ((result = lex(source, &offset, &token)) == LEX_OK) {
    printf("%-3zu %-10s `%.*s`\n", token.offset, lexer_token_name(token.kind),
           (int)token.length, token.text);
  }
  if (result != LEX_EOF) {
    print_error(source, offset);
  }
}

void lexer_print_token_stream(TokenStream stream) {
  for (size_t i = 0; i < stream.length; i++) {
    Token token = stream.data[i];
    printf("%-3zu %-10s `%.*s`\n", token.offset, lexer_token_name(token.kind),
           (int)token.length, token.text);
  }
}

bool peek_token(TokenStream stream, size_t token_index, Token *out) {
  if (token_index >= stream.length) {
    return false;
  }
  *out = stream.data[token_index];
  return true;
}
