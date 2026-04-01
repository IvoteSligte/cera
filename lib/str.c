#include <string.h>
#include <stdbool.h>

#include "alloc.h"
#include "log.h"
#include "str.h"

String from_c_string(char *c_string) {
  return (String){
      .len = strlen(c_string),
      .data = c_string,
  };
}

char *to_c_string(const String string) {
  char *c_string = (char *)allocate(string.len + 1);
  memcpy(c_string, string.data, string.len);
  return c_string;
}

char str_at(String string, size_t i) {
  if (i < 0)
    panic("str_at index < 0: %zu.", i);
  if (i >= string.len)
    panic("str_at index >= len: %zu >= %zu.", i, string.len);

  return string.data[i];
}

void str_push(String* string, char c) {
  String new_string = {
      .data = allocate(string->len + 1),
      .len = string->len + 1,
  };
  memcpy(new_string.data, string->data, string->len);
  free_memory(string->data);
  *string = new_string;  
}

bool is_lower_alpha(char c) { return c >= 'a' && c <= 'z'; }
bool is_upper_alpha(char c) { return c >= 'A' && c <= 'Z'; }
bool is_alpha(char c) { return is_lower_alpha(c) || is_upper_alpha(c); }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_alnum(char c) { return is_alpha(c) || is_digit(c); }
bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\f' || c == '\r';
}
