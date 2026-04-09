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

bool lex(const char *source, size_t *offset, Token *out) {
  size_t longest_match = 0;

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
  return longest_match != 0;
}

void lexer_print_tokens(const char *source) {
  size_t offset = 0;
  Token token;

  while (lex(source, &offset, &token)) {
    printf("%s `%.*s`\n", lexer_token_name(token.kind), (int)token.length,
           token.text);
  }
}
