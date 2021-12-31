#pragma once

#include <stdint.h>
#include <vec/vec.h>

#include "cdecl/string.h"

#define CDECL_TOKEN_TYPE_VARIANTS_ \
  X(Array)                         \
  X(As)                            \
  X(Cast)                          \
  X(Comma)                         \
  X(Declare)                       \
  X(DoubleColon)                   \
  X(Explain)                       \
  X(Function)                      \
  X(Block)                         \
  X(Help)                          \
  X(Into)                          \
  X(Of)                            \
  X(Member)                        \
  X(Pointer)                       \
  X(Reference)                     \
  X(Returning)                     \
  X(Set)                           \
  X(To)                            \
                                   \
  X(Char)                          \
  X(Class)                         \
  X(ConstVolatile)                 \
  X(Double)                        \
  X(Enum)                          \
  X(Float)                         \
  X(Int)                           \
  X(Long)                          \
  X(Name)                          \
  X(Number)                        \
  X(Short)                         \
  X(Signed)                        \
  X(Struct)                        \
  X(Union)                         \
  X(Unsigned)                      \
  X(Void)                          \
  X(Auto)                          \
  X(Extern)                        \
  X(Register)                      \
  X(Static)                        \
                                   \
  X(Ampersand)                     \
  X(Asterisk)                      \
  X(LeftBracket)                   \
  X(RightBracket)                  \
  X(LeftParen)                     \
  X(RightParen)                    \
  X(Semicolon)                     \
  X(Caret)                         \
  X(Newline)                       \
  X(Exit)                          \
  X(Quit)                          \
                                   \
  X(Eof)                           \
  X(Mismatch)

typedef enum {
#define X(x) kCdeclTokenType##x,
  CDECL_TOKEN_TYPE_VARIANTS_
#undef X
} CdeclTokenType;

typedef struct {
  CdeclTokenType type;
  String text;
} CdeclToken;

typedef VEC_TYPE(CdeclToken) CdeclTokens;

CdeclTokens CdeclTokenize(String text);
void CdeclTokensCleanup(CdeclTokens* tokens);
