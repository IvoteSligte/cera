#include "lexer.h"
#include "offset.h"
#include "regexp.h"

typedef struct {
  TokenKind kind;
  const char *name;
  const char *display_name;
  const char *regex;
} Matcher;

#define M(name, display_name, regex) {t##name, #name, display_name, "^" regex}

// assumes that tokens are in the same order as in the TokenKind definition
const Matcher MATCHERS[] = {
    M(EOF, "EOF", "[><]"),
    M(WHITESPACE, "whitespace", "[\n\t ]+"),
    M(COMMENT, "comment", "// .*\n"),
    // words
    M(IDENT, "identifier", "[a-zA-Z_][a-zA-Z_0-9]*"),
    M(NUMBER, "number", "[0-9]+"),
    M(STRING, "string", "\"([^\\\\\"]|\\\\.)*\""),
    // symbols
    M(LPAREN, "(", "\\("),
    M(RPAREN, ")", "\\)"),
    M(LBRACE, "{", "\\{"),
    M(RBRACE, "}", "\\}"),
    M(LBRACKET, "[", "\\["),
    M(RBRACKET, "]", "]"),
    // operators
    M(PLUS, "+", "\\+"),
    M(MINUS, "-", "-"),
    M(STAR, "*", "\\*"),
    M(SLASH, "/", "/"),
    M(PLUS_EQ, "+=", "\\+="),
    M(MINUS_EQ, "-=", "-="),
    M(STAR_EQ, "*=", "\\*="),
    M(SLASH_EQ, "/=", "/="),
    M(LT, "<", "<"),
    M(GT, ">", ">"),
    M(LT_EQ, "<=", "<="),
    M(GT_EQ, ">=", ">="),
    M(EQ_EQ, "==", "=="),
    M(AMP_AMP, "&&", "&&"),
    M(BAR_BAR, "||", "\\|\\|"),
    // misc symbols
    M(HASHTAG, "#", "#"),
    M(SEMI, ";", ";"),
    M(COMMA, ",", ","),
    M(DOT, ".", "\\."),
    M(EQ, "=", "="),
    M(COL, ":", ":"),
    M(COL_EQ, ":=", ":="),
    M(COL_COL, "::", "::"),
    // keywords
    M(STRUCT, "struct", "struct"),
    M(UNION, "union", "union"),
    M(ENUM, "enum", "enum"),
    M(RETURN, "return", "return"),
    M(FOR, "for", "for"),
    M(IF, "if", "if"),
    M(WHILE, "while", "while"),
    M(TRUE, "true", "true"),
    M(FALSE, "false", "false"),
};
#undef M

#define NUM_MATCHERS (sizeof(MATCHERS) / sizeof(MATCHERS[0]))

regex_t regexes[NUM_MATCHERS];

void lexer_init(void) {
  assert(tKIND_COUNT == NUM_MATCHERS);
  for (size_t i = 0; i < NUM_MATCHERS; i++)
    compile_regex(MATCHERS[i].regex, &regexes[i]);
}

void lexer_free(void) {
  for (size_t i = 0; i < NUM_MATCHERS; i++) {
    regfree(&regexes[i]);
  }
}

const char *token_name(TokenKind kind) { return MATCHERS[kind].name; }

const char *token_display_name(TokenKind kind) {
  return MATCHERS[kind].display_name;
}

int token_precedence(TokenKind kind) {
  switch (kind) {
  case tBAR_BAR:
    return 0;
  case tAMP_AMP:
    return 1;
  case tLT:
  case tGT:
  case tLT_EQ:
  case tGT_EQ:
    return 2;
  case tPLUS:
  case tMINUS:
    return 3;
  case tSTAR:
  case tSLASH:
    return 4;
  default:
    panicf("Tried to retrieve precedence of non-operator: `%s`",
           token_display_name(kind));
    break;
  }
}

LexResult lex(const char *source, size_t *offset, Token *out,
              LexError *error_data) {
  size_t longest_match = 0;

  if (*offset >= strlen(source)) {
    return LEX_EOF;
  }
  for (size_t i = 0; i < NUM_MATCHERS; i++) {
    regmatch_t match;
    if (match_regex(&regexes[i], &source[*offset], &match)) {
      *out = (Token){
          .offset = *offset,
          .length = match.rm_eo - match.rm_so, // end - start
          .text = &source[*offset],
          .kind = MATCHERS[i].kind,
      };
      longest_match = out->length;
    }
  }
  *offset += longest_match;
  if (longest_match == 0) {
    return LEX_NO_MATCH;
  } else if (out->kind == tWHITESPACE || out->kind == tCOMMENT) {
    return lex(source, offset, out, error_data); // skip whitespace and comments
  } else {
    return LEX_OK;
  }
}

void print_lex_error(LexError error) {
  OffsetInfo oi = get_offset_info(error.source, error.offset);
  int length = 0;
  // find first whitespace after offset
  for (; !IS_ONE_OF(error.source[error.offset + length], '\0', '\n', ' ');
       length++)
    ;
  eprintf(
      "Error: Failed to match token at line %zu, column %zu. String: `%.*s`\n",
      oi.line_number, oi.column_number, length, &error.source[error.offset]);
}

bool fill_token_stream(const char *source, TokenStream *out,
                       LexError *error_data) {
  size_t offset = 0;
  Token token = {0};
  LexResult result = {0};
  *out = (TokenStream){.source = source};
  *error_data = (LexError){.source = source};

  while ((result = lex(source, &offset, &token, error_data)) == LEX_OK) {
    out->data = realloc(out->data, sizeof(Token) * (out->length + 1));
    out->data[out->length] = token;
    out->length++;
  }
  if (result != LEX_EOF) {
    error_data->offset = offset;
    return false;
  }
  return true;
}

void free_token_stream(TokenStream *stream) {
  free(stream->data);
  stream->length = 0;
}

void print_token_stream(TokenStream stream) {
  for (size_t i = 0; i < stream.length; i++) {
    Token token = stream.data[i];
    printf("%-3zu %-10s `%.*s`\n", token.offset, token_name(token.kind),
           (int)token.length, token.text);
  }
}

Token get_token(TokenStream stream, size_t token_index) {
  if (token_index >= stream.length) {
    return (Token){.kind = tEOF};
  }
  return stream.data[token_index];
}
