#pragma once

#include <stdint.h>
#include <vec/vec.h>

#include "cdecl/string.h"

typedef enum {
  kCdeclTokenTypeArray,
  kCdeclTokenTypeAs,
  kCdeclTokenTypeCast,
  kCdeclTokenTypeComma,
  kCdeclTokenTypeDeclare,
  kCdeclTokenTypeDoubleColon,
  kCdeclTokenTypeExplain,
  kCdeclTokenTypeFunction,
  kCdeclTokenTypeBlock,
  kCdeclTokenTypeHelp,
  kCdeclTokenTypeInto,
  kCdeclTokenTypeOf,
  kCdeclTokenTypeMember,
  kCdeclTokenTypePointer,
  kCdeclTokenTypeReference,
  kCdeclTokenTypeReturning,
  kCdeclTokenTypeSet,
  kCdeclTokenTypeTo,

  kCdeclTokenTypeChar,
  kCdeclTokenTypeClass,
  kCdeclTokenTypeConstVolatile,
  kCdeclTokenTypeDouble,
  kCdeclTokenTypeEnum,
  kCdeclTokenTypeFloat,
  kCdeclTokenTypeInt,
  kCdeclTokenTypeLong,
  kCdeclTokenTypeName,
  kCdeclTokenTypeNumber,
  kCdeclTokenTypeShort,
  kCdeclTokenTypeSigned,
  kCdeclTokenTypeStruct,
  kCdeclTokenTypeUnion,
  kCdeclTokenTypeUnsigned,
  kCdeclTokenTypeVoid,
  kCdeclTokenTypeAuto,
  kCdeclTokenTypeExtern,
  kCdeclTokenTypeRegister,
  kCdeclTokenTypeStatic,

  kCdeclTokenTypeAmpersand,
  kCdeclTokenTypeAsterisk,
  kCdeclTokenTypeLeftBracket,
  kCdeclTokenTypeRightBracket,
  kCdeclTokenTypeLeftParen,
  kCdeclTokenTypeRightParen,
  kCdeclTokenTypeSemicolon,
  kCdeclTokenTypeCaret,
  kCdeclTokenTypeNewline,
  kCdeclTokenTypeExit,
  kCdeclTokenTypeQuit,
} CdeclTokenType;

typedef struct {
  CdeclTokenType type;
  String text;
} CdeclToken;

typedef VEC_TYPE(CdeclToken) CdeclTokens;

CdeclTokens CdeclTokenize(String text);
void CdeclTokensCleanup(CdeclTokens* tokens);
