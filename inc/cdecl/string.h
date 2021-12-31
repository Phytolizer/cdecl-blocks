#pragma once

#include <vec/vec.h>

typedef VEC_TYPE(char) String;

String StringFromChar(char c);
String StringDup(const char* s);
