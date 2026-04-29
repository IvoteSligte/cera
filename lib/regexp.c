
#include "regex.h"
#include "util.h"
#include <regex.h>

void compile_regex(const char *pattern, regex_t *regex) {
  int result = regcomp(regex, pattern, REG_EXTENDED);
  if (result) {
    char errbuf[100];
    regerror(result, regex, errbuf, 100);
    panicf("Failed to compile regex `%s`. Error: %s", pattern, errbuf);
  }
}

bool match_regex(regex_t *regex, const char *string, regmatch_t *out_match) {
  int result = regexec(regex, string, 1, out_match, 0);
  if (!IS_ONE_OF(result, REG_NOERROR, REG_NOMATCH)) {
    char errbuf[100];
    regerror(result, regex, errbuf, 100);
    panicf("Failed to run regex. Error: %s", errbuf);
  }
  return result == REG_NOERROR; // match
}

void free_regex(regex_t* regex) {
  regfree(regex);
}
