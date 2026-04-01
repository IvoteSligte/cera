#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 10000
#define INDENT_STACK_SIZE 256

typedef enum {
  TOKEN_IDENT,
  TOKEN_STRING,

  TOKEN_ARROW,    // ->
  TOKEN_LPAREN,   // (
  TOKEN_RPAREN,   // )
  TOKEN_STAR,     // *
  TOKEN_PLUS,     // +
  TOKEN_QUESTION, // ?
  TOKEN_COLON,    // :
  TOKEN_EQUAL,    // =
  TOKEN_COMMA,    // ,
  TOKEN_LBRACKET, // [
  TOKEN_RBRACKET, // ]

  TOKEN_NEWLINE,
  TOKEN_INDENT,
  TOKEN_DEDENT,

  TOKEN_EOF
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

Token tokens[MAX_TOKENS];
int token_count = 0;

void add_token(TokenType type, const char *value) {
  tokens[token_count].type = type;
  tokens[token_count].value = value ? strdup(value) : NULL;
  token_count++;
}

int indent_stack[INDENT_STACK_SIZE];
int indent_top = 0;

void push_indent(int n) { indent_stack[++indent_top] = n; }

int pop_indent() { return indent_stack[indent_top--]; }

int current_indent() { return indent_stack[indent_top]; }

int is_identifier_start(char c) { return isalpha(c); }

int is_identifier_char(char c) { return isalnum(c); }

char *read_identifier(const char *src, int *i) {
  int start = *i;
  while (is_identifier_char(src[*i]))
    (*i)++;
  int len = *i - start;

  char *str = (char *)malloc(len + 1);
  strncpy(str, &src[start], len);
  str[len] = '\0';
  return str;
}

char *read_string(const char *src, int *i) {
  (*i)++; // skip opening quote
  int start = *i;

  while (src[*i] && src[*i] != '"')
    (*i)++;

  int len = *i - start;
  char *str = (char *)malloc(len + 1);
  strncpy(str, &src[start], len);
  str[len] = '\0';

  if (src[*i] == '"')
    (*i)++; // skip closing quote

  return str;
}

void handle_indentation(int indent) {
  int current = current_indent();

  if (indent > current) {
    push_indent(indent);
    add_token(TOKEN_INDENT, NULL);
  } else {
    while (indent < current) {
      pop_indent();
      add_token(TOKEN_DEDENT, NULL);
      current = current_indent();
    }
  }
}

void tokenize(const char *src) {
  int i = 0;
  int at_line_start = 1;

  indent_stack[0] = 0;

  while (src[i]) {
    char c = src[i];

    // Handle indentation at line start
    if (at_line_start) {
      int indent = 0;
      while (src[i] == ' ' || src[i] == '\t') {
        indent += (src[i] == ' ') ? 1 : 4;
        i++;
      }

      handle_indentation(indent);
      at_line_start = 0;
      continue;
    }

    // Newline
    if (c == '\n') {
      add_token(TOKEN_NEWLINE, NULL);
      i++;
      at_line_start = 1;
      continue;
    }

    // Skip whitespace
    if (isspace(c)) {
      i++;
      continue;
    }

    // IDENT
    if (is_identifier_start(c)) {
      char *id = read_identifier(src, &i);
      add_token(TOKEN_IDENT, id);
      free(id);
      continue;
    }

    // STRING
    if (c == '"') {
      char *str = read_string(src, &i);
      add_token(TOKEN_STRING, str);
      free(str);
      continue;
    }

    // Symbols
    if (c == '-' && src[i + 1] == '>') {
      add_token(TOKEN_ARROW, "->");
      i += 2;
      continue;
    }

    switch (c) {
    case '(':
      add_token(TOKEN_LPAREN, "(");
      break;
    case ')':
      add_token(TOKEN_RPAREN, ")");
      break;
    case '[':
      add_token(TOKEN_LBRACKET, "[");
      break;
    case ']':
      add_token(TOKEN_RBRACKET, "]");
      break;
    case '*':
      add_token(TOKEN_STAR, "*");
      break;
    case '+':
      add_token(TOKEN_PLUS, "+");
      break;
    case '?':
      add_token(TOKEN_QUESTION, "?");
      break;
    case ':':
      add_token(TOKEN_COLON, ":");
      break;
    case '=':
      add_token(TOKEN_EQUAL, "=");
      break;
    case ',':
      add_token(TOKEN_COMMA, ",");
      break;

    default:
      printf("Unexpected character: %c\n", c);
      exit(1);
    }

    i++;
  }

  // Final dedents
  while (indent_top > 0) {
    pop_indent();
    add_token(TOKEN_DEDENT, NULL);
  }

  add_token(TOKEN_EOF, NULL);
}

const char *token_type_name(TokenType type) {
  switch (type) {
  case TOKEN_IDENT:
    return "IDENT";
  case TOKEN_STRING:
    return "STRING";
  case TOKEN_ARROW:
    return "->";
  case TOKEN_LPAREN:
    return "(";
  case TOKEN_RPAREN:
    return ")";
  case TOKEN_LBRACKET:
    return "[";
  case TOKEN_RBRACKET:
    return "]";
  case TOKEN_STAR:
    return "*";
  case TOKEN_PLUS:
    return "+";
  case TOKEN_QUESTION:
    return "?";
  case TOKEN_COLON:
    return ":";
  case TOKEN_EQUAL:
    return "=";
  case TOKEN_COMMA:
    return ",";
  case TOKEN_NEWLINE:
    return "NEWLINE";
  case TOKEN_INDENT:
    return "INDENT";
  case TOKEN_DEDENT:
    return "DEDENT";
  case TOKEN_EOF:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}

int main() {
  const char *source = "tokens = \n"
                       "    ID = [A-Za-z][A-Za-z0-9]*\n"
                       "    skip WHITESPACE = [ \t\n\f]\n";

  tokenize(source);

  for (int i = 0; i < token_count; i++) {
    printf("%s", token_type_name(tokens[i].type));
    if (tokens[i].value)
      printf("(%s)", tokens[i].value);
    printf("\n");
  }

  return 0;
}
