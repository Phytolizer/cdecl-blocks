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
  AddKeyword(&keywords, StringFromCstr("array"), kCdeclTokenTypeArray);
  AddKeyword(&keywords, StringFromCstr("as"), kCdeclTokenTypeAs);
  AddKeyword(&keywords, StringFromCstr("cast"), kCdeclTokenTypeCast);
  AddKeyword(&keywords, StringFromCstr("declare"), kCdeclTokenTypeDeclare);
  AddKeyword(&keywords, StringFromCstr("exit"), kCdeclTokenTypeExit);
  AddKeyword(&keywords, StringFromCstr("help"), kCdeclTokenTypeHelp);
  AddKeyword(&keywords, StringFromCstr("into"), kCdeclTokenTypeInto);
  AddKeyword(&keywords, StringFromCstr("of"), kCdeclTokenTypeOf);
  AddKeyword(&keywords, StringFromCstr("explain"), kCdeclTokenTypeExplain);
  AddKeyword(&keywords, StringFromCstr("function"), kCdeclTokenTypeFunction);
  AddKeyword(&keywords, StringFromCstr("func"), kCdeclTokenTypeFunction);
  AddKeyword(&keywords, StringFromCstr("block"), kCdeclTokenTypeBlock);
  AddKeyword(&keywords, StringFromCstr("member"), kCdeclTokenTypeMember);
  AddKeyword(&keywords, StringFromCstr("pointer"), kCdeclTokenTypePointer);
  AddKeyword(&keywords, StringFromCstr("ptr"), kCdeclTokenTypePointer);
  AddKeyword(&keywords, StringFromCstr("quit"), kCdeclTokenTypeQuit);
  AddKeyword(&keywords, StringFromCstr("reference"), kCdeclTokenTypeReference);
  AddKeyword(&keywords, StringFromCstr("ref"), kCdeclTokenTypeReference);
  AddKeyword(&keywords, StringFromCstr("returning"), kCdeclTokenTypeReturning);
  AddKeyword(&keywords, StringFromCstr("ret"), kCdeclTokenTypeReturning);
  AddKeyword(&keywords, StringFromCstr("set"), kCdeclTokenTypeSet);
  AddKeyword(&keywords, StringFromCstr("to"), kCdeclTokenTypeTo);
  AddKeyword(&keywords, StringFromCstr("vector"), kCdeclTokenTypeArray);
  AddKeyword(&keywords, StringFromCstr("auto"), kCdeclTokenTypeAuto);
  AddKeyword(&keywords, StringFromCstr("character"), kCdeclTokenTypeChar);
  AddKeyword(&keywords, StringFromCstr("char"), kCdeclTokenTypeChar);
  AddKeyword(&keywords, StringFromCstr("class"), kCdeclTokenTypeClass);
  AddKeyword(&keywords, StringFromCstr("constant"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringFromCstr("const"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringFromCstr("double"), kCdeclTokenTypeDouble);
  AddKeyword(&keywords, StringFromCstr("enumeration"), kCdeclTokenTypeEnum);
  AddKeyword(&keywords, StringFromCstr("enum"), kCdeclTokenTypeEnum);
  AddKeyword(&keywords, StringFromCstr("external"), kCdeclTokenTypeExtern);
  AddKeyword(&keywords, StringFromCstr("float"), kCdeclTokenTypeFloat);
  AddKeyword(&keywords, StringFromCstr("integer"), kCdeclTokenTypeInt);
  AddKeyword(&keywords, StringFromCstr("int"), kCdeclTokenTypeInt);
  AddKeyword(&keywords, StringFromCstr("long"), kCdeclTokenTypeLong);
  AddKeyword(&keywords, StringFromCstr("noalias"), kCdeclTokenTypeConstVolatile);
  AddKeyword(&keywords, StringFromCstr("register"), kCdeclTokenTypeRegister);
  AddKeyword(&keywords, StringFromCstr("short"), kCdeclTokenTypeShort);
  AddKeyword(&keywords, StringFromCstr("signed"), kCdeclTokenTypeSigned);
  AddKeyword(&keywords, StringFromCstr("static"), kCdeclTokenTypeStatic);
  AddKeyword(&keywords, StringFromCstr("structure"), kCdeclTokenTypeStruct);
  AddKeyword(&keywords, StringFromCstr("struct"), kCdeclTokenTypeStruct);
  AddKeyword(&keywords, StringFromCstr("union"), kCdeclTokenTypeUnion);
  AddKeyword(&keywords, StringFromCstr("unsigned"), kCdeclTokenTypeUnsigned);
  AddKeyword(&keywords, StringFromCstr("void"), kCdeclTokenTypeVoid);
  AddKeyword(&keywords, StringFromCstr("volatile"), kCdeclTokenTypeConstVolatile);
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
                                  .text = StringFromCstr("::"),
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
  for (uint64_t i = 0; i < tokens->length; ++i) {
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
