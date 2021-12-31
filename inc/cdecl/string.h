#pragma once

#include <vec/vec.h>

typedef VEC_TYPE(char) String;

String StringFromChar(char c);
String StringFromCstr(const char* s);
String StringDup(String s);
String StringConcat(String s1, ...);
