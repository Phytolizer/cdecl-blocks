#include "cdecl/cdlex.h"

#include <stdbool.h>
#include <stdint.h>

static bool IsIdentifierStart(char c);
static bool IsIdentifierPart(char c);

CdeclTokens CdeclTokenize(String text) {
  CdeclTokens tokens = {0};
  String token_text = {0};
  uint64_t i = 0;
  char keep_char = '\0';
  while (i < text.length) {
    keep_char = text.data[i];
    if (IsIdentifierStart(keep_char)) {
      while (IsIdentifierPart(keep_char)) {
        VEC_PUSH(&token_text, keep_char);
        i++;
        keep_char = text.data[i];
      }
      VEC_PUSH(&tokens, ((CdeclToken){
                            .type = kCdeclTokenTypeName,
                            .text = token_text,
                        }));
      // ownership of token_text is transferred to the tokens vector, don't free
      token_text = (String){0};
    }
  }

  return tokens;
}

bool IsIdentifierStart(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

bool IsIdentifierPart(char c) {
  return IsIdentifierStart(c) || (c >= '0' && c <= '9');
}
