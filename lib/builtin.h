#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char *text;
  int64_t length;
} CeamString;

void print_bool(bool b);
void print_int(int64_t n);
void print_string(CeamString string);
void print_char(uint32_t c);
void print_byte(uint8_t b);
bool __string_eq(CeamString left, CeamString right);
