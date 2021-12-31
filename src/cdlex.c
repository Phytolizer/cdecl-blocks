#include "cdecl/cdlex.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uthash.h>

typedef struct {
  String buffer;
  uint64_t position;
} LookaheadBuffer;

typedef struct {
  char* text;
  CdeclTokenType type;
  UT_hash_handle hh;
} KeywordEntry;

static bool IsIdentifierStart(char c);
static bool IsIdentifierPart(char c);
static bool IsDigit(char c);
void LookAhead(LookaheadBuffer* buffer, String text, uint64_t index);
void LookFlush(LookaheadBuffer* buffer);
void ReportBadCharacter(String text, uint64_t index);
void AddKeyword(KeywordEntry** keywords, String text, CdeclTokenType type);

CdeclTokens CdeclTokenize(String text) {
  CdeclTokens tokens = {0};
  String token_text = {0};
  LookaheadBuffer lookahead = {0};
  KeywordEntry* keywords = NULL;
  AddKeyword(&keywords, StringDup("array"), kCdeclTokenTypeArray);
  AddKeyword(&keywords, StringDup("as"), kCdeclTokenTypeAs);
  AddKeyword(&keywords, StringDup("cast"), kCdeclTokenTypeCast);
  AddKeyword(&keywords, StringDup("declare"), kCdeclTokenTypeDeclare);
  AddKeyword(&keywords, StringDup("exit"), kCdeclTokenTypeExit);
  AddKeyword(&keywords, StringDup("help"), kCdeclTokenTypeHelp);
  AddKeyword(&keywords, StringDup("into"), kCdeclTokenTypeInto);
  AddKeyword(&keywords, StringDup("of"), kCdeclTokenTypeOf);
  AddKeyword(&keywords, StringDup("explain"), kCdeclTokenTypeExplain);
  AddKeyword(&keywords, StringDup("function"), kCdeclTokenTypeFunction);
  AddKeyword(&keywords, StringDup("func"), kCdeclTokenTypeFunction);
  AddKeyword(&keywords, StringDup("block"), kCdeclTokenTypeBlock);
  AddKeyword(&keywords, StringDup("member"), kCdeclTokenTypeMember);
  AddKeyword(&keywords, StringDup("pointer"), kCdeclTokenTypePointer);
  AddKeyword(&keywords, StringDup("ptr"), kCdeclTokenTypePointer);
  AddKeyword(&keywords, StringDup("quit"), kCdeclTokenTypeQuit);
  AddKeyword(&keywords, StringDup("reference"), kCdeclTokenTypeReference);
  AddKeyword(&keywords, StringDup("ref"), kCdeclTokenTypeReference);
  AddKeyword(&keywords, StringDup("returning"), kCdeclTokenTypeReturning);
  AddKeyword(&keywords, StringDup("ret"), kCdeclTokenTypeReturning);
  AddKeyword(&keywords, StringDup("set"), kCdeclTokenTypeSet);
  AddKeyword(&keywords, StringDup("to"), kCdeclTokenTypeTo);
  AddKeyword(&keywords, StringDup("vector"), kCdeclTokenTypeArray);
  AddKeyword(&keywords, StringDup("auto"), kCdeclTokenTypeAuto);
  AddKeyword(&keywords, StringDup("character"), kCdeclTokenTypeChar);
  AddKeyword(&keywords, StringDup("char"), kCdeclTokenTypeChar);
  AddKeyword(&keywords, StringDup("class"), kCdeclTokenTypeClass);
  AddKeyword(&keywords, StringDup("constant"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringDup("const"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringDup("double"), kCdeclTokenTypeDouble);
  AddKeyword(&keywords, StringDup("enumeration"), kCdeclTokenTypeEnum);
  AddKeyword(&keywords, StringDup("enum"), kCdeclTokenTypeEnum);
  AddKeyword(&keywords, StringDup("external"), kCdeclTokenTypeExtern);
  AddKeyword(&keywords, StringDup("float"), kCdeclTokenTypeFloat);
  AddKeyword(&keywords, StringDup("integer"), kCdeclTokenTypeInt);
  AddKeyword(&keywords, StringDup("int"), kCdeclTokenTypeInt);
  AddKeyword(&keywords, StringDup("long"), kCdeclTokenTypeLong);
  AddKeyword(&keywords, StringDup("noalias"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringDup("register"), kCdeclTokenTypeRegister);
  AddKeyword(&keywords, StringDup("short"), kCdeclTokenTypeShort);
  AddKeyword(&keywords, StringDup("signed"), kCdeclTokenTypeSigned);
  AddKeyword(&keywords, StringDup("static"), kCdeclTokenTypeStatic);
  AddKeyword(&keywords, StringDup("structure"), kCdeclTokenTypeStruct);
  AddKeyword(&keywords, StringDup("struct"), kCdeclTokenTypeStruct);
  AddKeyword(&keywords, StringDup("union"), kCdeclTokenTypeUnion);
  AddKeyword(&keywords, StringDup("unsigned"), kCdeclTokenTypeUnsigned);
  AddKeyword(&keywords, StringDup("void"), kCdeclTokenTypeVoid);
  AddKeyword(&keywords, StringDup("volatile"), kCdeclTokenTypeConstVolatile);
  // TODO(phyto): Add keywords.
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
      VEC_PUSH(&token_text, '\0');
      KeywordEntry* kw = NULL;
      HASH_FIND_STR(keywords, token_text.data, kw);
      VEC_POP(&token_text);
      if (kw != NULL) {
        VEC_PUSH(&tokens, ((CdeclToken){
                              .type = kw->type,
                              .text = token_text,
                          }));
      } else {
        VEC_PUSH(&tokens, ((CdeclToken){
                              .type = kCdeclTokenTypeName,
                              .text = token_text,
                          }));
      }
      token_text = (String){0};
    } else if (IsDigit(keep_char)) {
      while (IsDigit(keep_char)) {
        VEC_PUSH(&token_text, keep_char);
        i++;
        keep_char = text.data[i];
      }
      VEC_PUSH(&tokens, ((CdeclToken){
                            .type = kCdeclTokenTypeNumber,
                            .text = token_text,
                        }));
      token_text = (String){0};
    } else {
      switch (keep_char) {
        case '#':
          // comment
          while (keep_char != '\n' && i < text.length) {
            i++;
            keep_char = text.data[i];
          }
          break;
        case '\t':
        case ' ':
          // whitespace
          i++;
          break;
        case '&':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeAmpersand,
                                .text = StringFromChar('&'),
                            }));
          i++;
          break;
        case '*':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeAsterisk,
                                .text = StringFromChar('*'),
                            }));
          i++;
          break;
        case '[':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeLeftBracket,
                                .text = StringFromChar('['),
                            }));
          i++;
          break;
        case ']':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeRightBracket,
                                .text = StringFromChar(']'),
                            }));
          i++;
          break;
        case '(':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeLeftParen,
                                .text = StringFromChar('('),
                            }));
          i++;
          break;
        case ')':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeRightParen,
                                .text = StringFromChar(')'),
                            }));
          i++;
          break;
        case ';':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeSemicolon,
                                .text = StringFromChar(';'),
                            }));
          i++;
          break;
        case '^':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeCaret,
                                .text = StringFromChar('^'),
                            }));
          i++;
          break;
        case '\n':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeNewline,
                                .text = StringFromChar('\n'),
                            }));
          i++;
          break;
        case '?':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeHelp,
                                .text = StringFromChar('?'),
                            }));
          i++;
          break;
        case ',':
          VEC_PUSH(&tokens, ((CdeclToken){
                                .type = kCdeclTokenTypeComma,
                                .text = StringFromChar(','),
                            }));
          i++;
          break;
        case ':':
          LookAhead(&lookahead, text, i);
          if (lookahead.buffer.data[0] == ':') {
            VEC_PUSH(&tokens, ((CdeclToken){
                                  .type = kCdeclTokenTypeDoubleColon,
                                  .text = StringDup("::"),
                              }));
            LookFlush(&lookahead);
            i += 2;
          } else {
            ReportBadCharacter(text, i);
            i++;
          }
          break;
        default:
          ReportBadCharacter(text, i);
          i++;
      }
    }
  }

  KeywordEntry* kw;
  KeywordEntry* tmp;
  HASH_ITER(hh, keywords, kw, tmp) {
    HASH_DEL(keywords, kw);
    free(kw->text);
    free(kw);
  }
  return tokens;
}

void CdeclTokensCleanup(CdeclTokens* tokens) {
  for (size_t i = 0; i < tokens->length; ++i) {
    VEC_FREE(&tokens->data[i].text);
  }
  VEC_FREE(tokens);
  *tokens = (CdeclTokens){0};
}

bool IsIdentifierStart(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

bool IsIdentifierPart(char c) {
  return IsIdentifierStart(c) || (c >= '0' && c <= '9');
}

bool IsDigit(char c) {
  return c >= '0' && c <= '9';
}

void LookAhead(LookaheadBuffer* buffer, String text, uint64_t index) {
  if (buffer->position == buffer->buffer.length) {
    VEC_PUSH(&buffer->buffer, text.data[index]);
  } else {
    buffer->buffer.data[buffer->position] = text.data[index];
  }
  ++buffer->position;
}

void LookFlush(LookaheadBuffer* buffer) {
  buffer->position = 0;
}

void ReportBadCharacter(String text, uint64_t index) {
  printf("Bad character at index %" PRIu64 ": '%c'\n", index, text.data[index]);
}

void AddKeyword(KeywordEntry** keywords, String text, CdeclTokenType type) {
  KeywordEntry* entry = malloc(sizeof(KeywordEntry));
  entry->text = malloc(text.length + 1);
  memcpy(entry->text, text.data, text.length);
  entry->text[text.length] = '\0';
  entry->type = type;
  HASH_ADD_KEYPTR(hh, *keywords, entry->text, text.length, entry);
  VEC_FREE(&text);
}
