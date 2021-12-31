#include <stdio.h>

#include "cdecl/cdlex.h"
#include "cdecl/string.h"

int main(int argc, char** argv) {
  String text = StringDup(
      "declare bar as const pointer to array 5 of pointer to function (int) "
      "returning const pointer to char");
  CdeclTokens tokens = CdeclTokenize(text);
  for (uint64_t i = 0; i < tokens.length; i++) {
    printf("%2d: %.*s\n", tokens.data[i].type, (int)tokens.data[i].text.length,
           tokens.data[i].text.data);
  }
  CdeclTokensCleanup(&tokens);
  VEC_FREE(&text);
  return 0;
}
