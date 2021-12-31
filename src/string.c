#include "cdecl/string.h"

#include <stdarg.h>

String StringFromChar(char c) {
  String s = {0};
  VEC_PUSH(&s, c);
  return s;
}

String StringFromCstr(const char* s) {
  String str = {0};
  for (uint64_t i = 0; s[i] != '\0'; i++) {
    VEC_PUSH(&str, s[i]);
  }
  return str;
}

String StringDup(String s) {
  String str = {0};
  for (uint64_t i = 0; i < s.length; i++) {
    VEC_PUSH(&str, s.data[i]);
  }
  return str;
}

String StringConcat(String s1, ...) {
  String str = {0};
  va_list args;
  va_start(args, s1);
  for (String s = s1; s.data != NULL; s = va_arg(args, String)) {
    for (uint64_t i = 0; i < s.length; i++) {
      VEC_PUSH(&str, s.data[i]);
    }
  }
  return str;
}
