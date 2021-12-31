#include "cdecl/cdgram.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "cdecl/cdlex.h"
#include "cdecl/string.h"

typedef VEC_TYPE(CdeclTokenType) CdeclTokenTypes;

static const char* const kCdeclTokenTypes[] = {
#define X(x) #x,
    CDECL_TOKEN_TYPE_VARIANTS_
#undef X
};

typedef struct {
  CdeclTokens tokens;
  uint64_t position;
  CdeclTokenTypes expected;
  String message;
  CdeclCommand command;
  char prev;
} Parser;

typedef enum {
  kParseOk,
  kParseMismatch,
  kParseEof,
} ParseResult;

#define PARSER_CUR(P) ((P)->tokens.data[(P)->position].type)
#define PARSER_TEXT(P) ((P)->tokens.data[(P)->position].text)
#define PARSER_DUP_TEXT(P) StringDup(PARSER_TEXT(P))

static ParseResult ParseStatement(Parser* p);
static ParseResult ParseHelpStatement(Parser* p);
static ParseResult ParseDeclareStatement(Parser* p);
static ParseResult ParseCastStatement(Parser* p);
static ParseResult ParseExplainStatement(Parser* p);
static ParseResult ParseSetStatement(Parser* p);
static ParseResult ParseNewline(Parser* p);
static ParseResult ParseOptionalName(Parser* p, String* dest);
static ParseResult ParseCdecl(Parser* p, String* dest);
static ParseResult ParseOptionalStorage(Parser* p, String* dest);
static ParseResult ParseAdecl(Parser* p, Adecl* dest);
static void PrintWhatWasExpected(CdeclTokenTypes expected, String message);

void CdeclParse(CdeclTokens tokens) {
  Parser p = {
      .tokens = tokens,
      .position = 0,
      .expected = {0},
      .message = {0},
      .command = {0},
      .prev = '\0',
  };

  ParseResult result;
  while ((result = ParseStatement(&p)) == kParseOk) {
  }
  if (result == kParseMismatch) {
    PrintWhatWasExpected(p.expected, p.message);
  }
}

ParseResult ParseStatement(Parser* p) {
  if (p->position >= p->tokens.length) {
    return kParseEof;
  }

  switch (PARSER_CUR(p)) {
    case kCdeclTokenTypeHelp:
      p->position++;
      return ParseHelpStatement(p);
    case kCdeclTokenTypeDeclare:
      p->position++;
      return ParseDeclareStatement(p);
    case kCdeclTokenTypeCast:
      p->position++;
      return ParseCastStatement(p);
    case kCdeclTokenTypeExplain:
      p->position++;
      return ParseExplainStatement(p);
    case kCdeclTokenTypeSet:
      p->position++;
      return ParseSetStatement(p);
    case kCdeclTokenTypeNewline:
      p->position++;
      return kParseOk;
    default: {
      VEC_PUSH(&p->expected, kCdeclTokenTypeHelp);
      VEC_PUSH(&p->expected, kCdeclTokenTypeDeclare);
      VEC_PUSH(&p->expected, kCdeclTokenTypeCast);
      VEC_PUSH(&p->expected, kCdeclTokenTypeSet);
      VEC_PUSH(&p->expected, kCdeclTokenTypeNewline);
      return kParseMismatch;
    }
  }
}

ParseResult ParseHelpStatement(Parser* p) {
  p->command.which = kCdeclCommandHelp;
  return ParseNewline(p);
}

ParseResult ParseDeclareStatement(Parser* p) {
  p->command.which = kCdeclCommandDeclare;
  if (PARSER_CUR(p) == kCdeclTokenTypeName) {
    p->command.u.declare.name = PARSER_DUP_TEXT(p);
    p->position++;
    if (PARSER_CUR(p) != kCdeclTokenTypeAs) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeAs);
      return kParseMismatch;
    }
    p->position++;
  }
  ParseOptionalStorage(p, &p->command.u.declare.storage);
  if (ParseAdecl(p, &p->command.u.declare.adecl) != kParseOk) {
    return kParseMismatch;
  }
  return ParseNewline(p);
}

ParseResult ParseCastStatement(Parser* p) {
  p->command.which = kCdeclCommandCast;
  if (PARSER_CUR(p) == kCdeclTokenTypeName) {
    p->command.u.cast.name = PARSER_DUP_TEXT(p);
    p->position++;
    if (PARSER_CUR(p) != kCdeclTokenTypeInto) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeInto);
      return kParseMismatch;
    }
    p->position++;
  }
  if (ParseAdecl(p, &p->command.u.cast.adecl) != kParseOk) {
    return kParseMismatch;
  }
  return ParseNewline(p);
}

ParseResult ParseExplainStatement(Parser* p) {
  p->command.which = kCdeclCommandExplain;
  if (PARSER_CUR(p) == kCdeclTokenTypeLeftParen) {
    p->command.u.explain.form = kCdeclExplainFormC;
    p->position++;
    ParseOptionalConstVolatileList(p,
                                   &p->command.u.explain.const_volatile_list);
    if (ParseType(p, &p->command.u.explain.type) != kParseOk) {
      return kParseMismatch;
    }
    if (ParseCast(p, &p->command.u.explain.cast) != kParseOk) {
      return kParseMismatch;
    }
    if (PARSER_CUR(p) != kCdeclTokenTypeRightParen) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeRightParen);
      return kParseMismatch;
    }
    ParseOptionalName(p, &p->command.u.explain.name);
    return ParseNewline(p);
  }

  if (!ParseOptionalStorage(p, &p->command.u.explain.storage) &&
      !ParseOptionalConstVolatileList(
          p, &p->command.u.explain.const_volatile_list)) {
    // first form
    if (ParseType(p, &p->command.u.explain.type) != kParseOk) {
      return kParseMismatch;
    }
    ParseOptionalConstVolatileList(p,
                                   &p->command.u.explain.const_volatile_list2);
    if (ParseCdecl(p, &p->command.u.explain.cdecl) != kParseOk) {
      return kParseMismatch;
    }
    return ParseNewline(p);
  }

  // second form
  ParseOptionalStorage(p, &p->command.u.explain.storage);
  ParseOptionalConstVolatileList(p, &p->command.u.explain.const_volatile_list);
  if (ParseCdecl(p, &p->command.u.explain.cdecl) != kParseOk) {
    return kParseMismatch;
  }
  return ParseNewline(p);
}

ParseResult ParseSetStatement(Parser* p) {
  p->command.which = kCdeclCommandSet;
  ParseOptionalName(p, &p->command.u.set.name);
  return ParseNewline(p);
}

ParseResult ParseNewline(Parser* p) {
  if (PARSER_CUR(p) != kCdeclTokenTypeNewline) {
    VEC_PUSH(&p->expected, kCdeclTokenTypeNewline);
    return kParseMismatch;
  }
  p->position++;
  return kParseOk;
}

ParseResult ParseOptionalName(Parser* p, String* dest) {
  if (PARSER_CUR(p) == kCdeclTokenTypeName) {
    *dest = PARSER_DUP_TEXT(p);
    p->position++;
  }
  return kParseOk;
}

inline ParseResult ParseCdecl(Parser* p, String* dest) {
  if (ParseCdecl1(p, dest) == kParseOk) {
    return kParseOk;
  }
  if (PARSER_CUR(p) == kCdeclTokenTypeAsterisk) {
    String qual = {0};
    p->position++;
    ParseOptionalConstVolatileList(p, &qual);
    // null terminate qual to distinguish it from NULL if it's empty
    VEC_PUSH(&qual, '\0');
    String cdecl = {0};
    if (ParseCdecl(p, &cdecl) != kParseOk) {
      VEC_FREE(&qual);
      return kParseMismatch;
    }
    *dest = StringConcat(
        cdecl, qual,
        StringFromCstr(qual.length ? " pointer to " : "pointer to "),
        (String){0});
    p->prev = 'p';
    return kParseOk;
  }
  if (PARSER_CUR(p) == kCdeclTokenTypeName) {
    String name = PARSER_DUP_TEXT(p);
    p->position++;
    if (PARSER_CUR(p) != kCdeclTokenTypeDoubleColon) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeDoubleColon);
      VEC_FREE(&name);
      return kParseMismatch;
    }
    p->position++;
    if (PARSER_CUR(p) != kCdeclTokenTypeAsterisk) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeAsterisk);
      VEC_FREE(&name);
      return kParseMismatch;
    }
    p->position++;
    String cdecl = {0};
    if (ParseCdecl(p, &cdecl) != kParseOk) {
      VEC_FREE(&name);
      return kParseMismatch;
    }
    *dest = StringConcat(cdecl, StringFromCstr("pointer to member of class "),
                         name, StringFromCstr(" "), (String){0});
    p->prev = 'p';
  }
  if (PARSER_CUR(p) == kCdeclTokenTypeAmpersand) {
    p->position++;
    String qual = {0};
    ParseOptionalConstVolatileList(p, &qual);
    // null terminate qual to distinguish it from NULL if it's empty
    VEC_PUSH(&qual, '\0');
    String cdecl = {0};
    if (ParseCdecl(p, &cdecl) != kParseOk) {
      VEC_FREE(&qual);
      return kParseMismatch;
    }
    *dest = StringConcat(
        cdecl, qual,
        StringFromCstr(qual.length ? " reference to " : "reference to "),
        (String){0});
    p->prev = 'r';
  }
  VEC_PUSH(&p->expected, kCdeclTokenTypeName);
  VEC_PUSH(&p->expected, kCdeclTokenTypeAsterisk);
  VEC_PUSH(&p->expected, kCdeclTokenTypeAmpersand);
  return kParseMismatch;
}

ParseResult ParseOptionalStorage(Parser* p, String* dest) {
  if (PARSER_CUR(p) == kCdeclTokenTypeStatic ||
      PARSER_CUR(p) == kCdeclTokenTypeExtern ||
      PARSER_CUR(p) == kCdeclTokenTypeRegister ||
      PARSER_CUR(p) == kCdeclTokenTypeStatic) {
    *dest = PARSER_DUP_TEXT(p);
    p->position++;
  }
  return kParseOk;
}

ParseResult ParseAdecl(Parser* p, Adecl* dest) {
  if (PARSER_CUR(p) == kCdeclTokenTypeFunction) {
    ++p->position;
    if (PARSER_CUR(p) == kCdeclTokenTypeReturning) {
      ++p->position;
      Adecl adecl = {0};
      if (ParseAdecl(p, &adecl) != kParseOk) {
        return kParseMismatch;
      }
      *dest = (Adecl){
          .left = adecl.left,
          .right = StringConcat(StringFromCstr("()"), adecl.right, (String){0}),
          .type = adecl.type,
      };
      return kParseOk;
    }
    if (PARSER_CUR(p) == kCdeclTokenTypeLeftParen) {
      ++p->position;
      String adecl_list = {0};
      if (ParseAdeclList(p, &adecl_list) != kParseOk) {
        return kParseMismatch;
      }
      if (PARSER_CUR(p) != kCdeclTokenTypeRightParen) {
        VEC_PUSH(&p->expected, kCdeclTokenTypeRightParen);
        VEC_FREE(&adecl_list);
        return kParseMismatch;
      }
      ++p->position;
      if (PARSER_CUR(p) != kCdeclTokenTypeReturning) {
        VEC_PUSH(&p->expected, kCdeclTokenTypeReturning);
        VEC_FREE(&adecl_list);
        return kParseMismatch;
      }
      ++p->position;
      Adecl adecl = {0};
      if (ParseAdecl(p, &adecl) != kParseOk) {
        VEC_FREE(&adecl_list);
        return kParseMismatch;
      }
      *dest = (Adecl){
          .left = adecl.left,
          .type = adecl.type,
          .right = StringConcat(StringFromCstr("("), adecl_list,
                                StringFromCstr(")"), adecl.right, (String){0}),
      };
      p->prev = 'f';
      return kParseOk;
    }
    VEC_PUSH(&p->expected, kCdeclTokenTypeReturning);
    VEC_PUSH(&p->expected, kCdeclTokenTypeLeftParen);
    return kParseMismatch;
  }
  if (PARSER_CUR(p) == kCdeclTokenTypeArray) {
    ++p->position;
    String adims;
    if (ParseArrayDimensions(p, &adims) != kParseOk) {
      return kParseMismatch;
    }
    if (PARSER_CUR(p) != kCdeclTokenTypeOf) {
      VEC_PUSH(&p->expected, kCdeclTokenTypeOf);
      VEC_FREE(&adims);
      return kParseMismatch;
    }
    ++p->position;
    Adecl adecl = {0};
    if (ParseAdecl(p, &adecl) != kParseOk) {
      VEC_FREE(&adims);
      return kParseMismatch;
    }
    *dest = (Adecl){
        .left = adecl.left,
        .type = adecl.type,
        .right = StringConcat(adims, adecl.right, (String){0}),
    };
  }
}

void PrintWhatWasExpected(CdeclTokenTypes expected, String message) {
  printf("parse mismatch: %.*s\n", (int)message.length, message.data);
  printf("expected one of: ");
  for (uint64_t i = 0; i < expected.length; i++) {
    printf("%s ", kCdeclTokenTypes[expected.data[i]]);
  }
}
