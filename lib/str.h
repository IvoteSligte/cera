#pragma once

#include <stdlib.h>

typedef struct {
  size_t len;
  char* data;  
} String;

String from_c_string(char *c_string);
char* to_c_string(const String c_string);

char str_at(String string, size_t i);

void str_push(String* string, char c);

#define str_iter(string) for (size_t i = 0; i < string.len; i++)

bool is_lower_alpha(char c);
bool is_upper_alpha(char c);
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);
bool is_space(char c);
