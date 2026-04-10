#include <regex.h> // TODO: cross-platform

#include "lexer.h"

typedef struct {
  TokenKind kind;
  const char *name;
  const char *regex;
} Matcher;

const Matcher MATCHERS[] = {
    {tWHITESPACE, "WHITESPACE", "^[\n\t ]+"},
    {tCOMMENT, "COMMENT", "^// .*\n"},
    // words
    {tIDENT, "IDENT", "^[a-zA-Z][a-zA-Z0-9]*"},
    {tNUMBER, "NUMBER", "^[0-9]+"},
    {tSTRING, "STRING", "^\"[^\"]*\""},
    // symbols
    {tLPAREN, "LPAREN", "^\\("},
    {tRPAREN, "RPAREN", "^\\)"},
    {tLBRACE, "LBRACE", "^\\{"},
    {tRBRACE, "RBRACE", "^\\}"},
    {tLBRACKET, "LBRACKET", "^\\["},
    {tRBRACKET, "RBRACKET", "^]"},
    {tDOT, "DOT", "^\\."},
    {tPLUS, "PLUS", "^\\+"},
    {tMINUS, "MINUS", "^\\-"},
    {tSTAR, "STAR", "^\\*"},
    {tSLASH, "SLASH", "^/"},
    {tHASHTAG, "HASHTAG", "^#"},
    {tSEMI, "SEMI", "^;"},
    {tCOMMA, "COMMA", "^,"},
    {tEQ, "EQ", "^="},
    {tCOLONEQ, "COLONEQ", "^:="},
    // keywords
    {tSTRUCT, "STRUCT", "^struct"},
    {tFUNC, "FUNC", "^func"},
    {tUNION, "UNION", "^union"},
    {tENUM, "ENUM", "^enum"},
};
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
  return (longest_match == 0) ? LEX_NO_MATCH : LEX_OK;
}

static void print_error(const char *source, size_t offset) {
  eprintf("Failed to match token in string: `%.*s`\n", 100, &source[offset]);  
}

bool fill_token_stream(const char *source, TokenStream* out) {
  size_t offset = 0;
  Token token;
  LexResult result;
  *out = (TokenStream) {0};

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

void free_token_stream(TokenStream stream) {
  free(stream.data);
}

void lexer_print_tokens(const char *source) {
  size_t offset = 0;
  Token token;
  LexResult result;  

  while ((result = lex(source, &offset, &token)) == LEX_OK) {
    printf("%s `%.*s`\n", lexer_token_name(token.kind), (int)token.length,
           token.text);
  }
  if (result != LEX_EOF) {
    print_error(source, offset);
  }
}

bool peek_token(TokenStream stream, size_t token_index, Token *out) {
  if (token_index >= stream.length) {
    return false;
  }
  *out = stream.data[token_index];
  return true;
}
