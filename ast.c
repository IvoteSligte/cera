#include "ast.h"

Span token_span(Token token) {
  return (Span) { .offset = token.offset, .length = token.length };
}

Span join_spans(Span left, Span right) {
  return (Span) {
      .offset = left.offset,
      .length = right.offset - left.offset + right.length,
  };
}
