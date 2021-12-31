#pragma once

#include <stdint.h>

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
} CdeclTokenType;

typedef struct {
  CdeclTokenType type;
  String text;
} CdeclToken;

typedef struct {
  CdeclToken* tokens;
  uint64_t length;
  uint64_t capacity;
} CdeclTokens;

CdeclTokens CdeclTokenize(String text);
