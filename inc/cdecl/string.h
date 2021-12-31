#pragma once

#include <stdint.h>

typedef struct {
  char* text;
  uint64_t length;
  uint64_t capacity;
} String;

void StringAppendChar(String* s, char c);
void StringReserveSpace(String* s, uint64_t capacity);
