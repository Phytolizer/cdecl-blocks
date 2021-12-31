#include "cdecl/string.h"

#include <stdlib.h>
#include <string.h>

void StringAppendChar(String* s, char c) {
  if (s->length == s->capacity) {
    StringReserveSpace(s, s->capacity * 2);
  }
  s->text[s->length++] = c;
}

void StringReserveSpace(String* s, uint64_t capacity) {
  char* new_text = (char*)calloc(1, capacity);
  memcpy(new_text, s->text, s->length);
  free(s->text);
  s->text = new_text;
  s->capacity = capacity;
}
