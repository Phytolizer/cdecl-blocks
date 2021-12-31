#include "cdecl/string.h"

String StringFromChar(char c) {
  String s = {0};
  VEC_PUSH(&s, c);
  return s;
}

String StringDup(const char* s) {
  String str = {0};
  for (uint64_t i = 0; s[i] != '\0'; i++) {
    VEC_PUSH(&str, s[i]);
  }
  return str;
}
