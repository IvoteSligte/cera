#include "lexer.h"
#include "offset.h"

#include <ctype.h>

typedef struct {
  TokenKind kind;
  const char *name;
  const char *display_name;
  size_t (*function)(const char *text);
} Matcher;

static size_t lex_whitespace(const char *text) {
  size_t length = 0;
  while (IS_ONE_OF(text[length], '\n', '\t', '\f', '\v', ' ')) {
    length++;
  }
  return length;
}

static size_t lex_comment(const char *text) {
  if (text[0] != '/' || text[1] != '/')
    return 0;
  size_t length = 2;
  while (text[length] != '\0') {
    length++;
    if (text[length] == '\n')
      break;
  }
  return length;
}

static size_t lex_identifier(const char *text) {
  if (!isalpha(text[0]) && text[0] != '_')
    return 0;
  size_t length = 1;
  while (isalnum(text[length]) || text[length] == '_')
    length++;
  return length;
}

static size_t lex_number(const char *text) {
  size_t length = 0;
  while (isdigit(text[length]))
    length++;
  return length;
}

static size_t lex_string(const char *text) {
  if (text[0] != '"')
    return 0;
  size_t length = 1;
  while (true) {
    if (text[length] == '\0')
      return 0; // TODO: error `expected '"', but found EOF`
    if (text[length] == '"') {
      length++;
      break;
    }
    if (text[length] == '\\' && text[length + 1] != '\0')
      length += 2;
    else
      length++;
  }
  return length;
}

static size_t lex_keyword(const char *text, const char *keyword) {
  size_t length = strlen(keyword);
  if (strncmp(text, keyword, length) == 0)
    return length;
  else
    return 0;
}

static size_t lex_eof(const char *text) {
  UNUSED(text);
  return 0;
}

#define CM($name, $display_name, $function)                                    \
  {t##$name, #$name, $display_name, $function}
#define M($name, $display_name) CM($name, $display_name, NULL)

// assumes that tokens are in the same order as in the TokenKind definition
const Matcher MATCHERS[] = {
    CM(EOF, "EOF", lex_eof),
    CM(WHITESPACE, "whitespace", lex_whitespace),
    CM(COMMENT, "comment", lex_comment),
    // words
    CM(IDENT, "identifier", lex_identifier),
    CM(NUMBER, "number", lex_number),
    CM(STRING, "string", lex_string),
    // symbols
    M(LPAREN, "("),
    M(RPAREN, ")"),
    M(LBRACE, "{"),
    M(RBRACE, "}"),
    M(LBRACKET, "["),
    M(RBRACKET, "]"),
    // operators
    M(PLUS, "+"),
    M(MINUS, "-"),
    M(STAR, "*"),
    M(SLASH, "/"),
    M(PLUS_EQ, "+="),
    M(MINUS_EQ, "-="),
    M(STAR_EQ, "*="),
    M(SLASH_EQ, "/="),
    M(LT, "<"),
    M(GT, ">"),
    M(LT_EQ, "<="),
    M(GT_EQ, ">="),
    M(EQ_EQ, "=="),
    M(AMP_AMP, "&&"),
    M(BAR_BAR, "||"),
    // misc symbols
    M(HASHTAG, "#"),
    M(SEMI, ";"),
    M(COMMA, ","),
    M(DOT, "."),
    M(EQ, "="),
    M(COL, ":"),
    M(COL_EQ, ":="),
    M(COL_COL, "::"),
    M(RARROW, "->"),
    // keywords
    M(STRUCT, "struct"),
    M(UNION, "union"),
    M(ENUM, "enum"),
    M(RETURN, "return"),
    M(FOR, "for"),
    M(IF, "if"),
    M(ELSE, "else"),
    M(WHILE, "while"),
    M(TRUE, "true"),
    M(FALSE, "false"),
};
#undef M

#define NUM_MATCHERS (sizeof(MATCHERS) / sizeof(MATCHERS[0]))

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
  case tEQ_EQ:    
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
    const char *text = source + *offset;
    size_t match_length = MATCHERS[i].function != NULL
                              ? MATCHERS[i].function(text)
                              : lex_keyword(text, MATCHERS[i].display_name);
    // prioritises longer and later matches
    if (match_length >= longest_match) {
      *out = (Token){
          .offset = *offset,
          .length = match_length,
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

void get_lex_error_info(LexError error, char** out_message, size_t* out_line, size_t* out_column) {
  OffsetInfo oi = get_offset_info(error.source, error.offset);
  *out_line = oi.line_number;
  *out_column = oi.column_number;  
  *out_message = strdup("failed to match token");
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
