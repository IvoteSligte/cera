#pragma once

#include <regex.h> // TODO: cross-platform
#include <stdbool.h>

void compile_regex(const char *pattern, regex_t *regex);

bool match_regex(regex_t* regex, const char* string, regmatch_t* out_match);

void free_regex(regex_t* regex);
