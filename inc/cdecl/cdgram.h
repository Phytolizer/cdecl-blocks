#pragma once

#include "cdecl/cdlex.h"

typedef enum {
  kCdeclCommandHelp,
  kCdeclCommandDeclare,
  kCdeclCommandCast,
  kCdeclCommandExplain,
  kCdeclCommandSet,
  kCdeclCommandNop,
} CdeclCommandType;

typedef enum {
  kCdeclExplainFormD,
  kCdeclExplainFormC,
} CdeclExplainForm;

typedef struct {
  String left;
  String right;
  String type;
} Adecl;

typedef struct {
  CdeclCommandType which;
  union {
    struct {
      String name;
      String storage;
      Adecl adecl;
    } declare;
    struct {
      String name;
      Adecl adecl;
    } cast;
    struct {
      CdeclExplainForm form;
      String name;
      String storage;
      String const_volatile_list;
      String type;
      String const_volatile_list2;
      String cdecl;
      String cast;
    } explain;
    struct {
      String name;
    } set;
  } u;
} CdeclCommand;

void CdeclParse(CdeclTokens tokens);
