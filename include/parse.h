#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include "linked.h"

#define IMPLEMENTED_TOKENS 11
#define IMPLEMENTED_CONDITIONALS 6
#define IMPLEMENTED_TYPES 9
#define IMPLEMENTED_OPERATORS 6

enum ParsingTokenType{
    token_scopeStart,
    token_scopeEnd,
    token_argumentStart,
    token_argumentEnd,
    token_function,
    token_while,
    token_for,
    token_preprocessor,
    token_set,
    token_separator,
    token_end,
    token_type,
    token_conditional,
    token_number,
    token_operator,
    token_invalid,
    token_filestart,
    token_name
};

enum ConditionalTypes{
    cond_notequals,
    cond_less,
    cond_greater,
    cond_less_equal,
    cond_greater_equal
};

enum BuiltInTypes{
    type_i8,
    type_i16,
    type_i32,
    type_i64,
    type_ch,
    type_none,
    type_point,
    type_addr,
    type_arr
};

enum OperatorTypes{
    op_inc, op_dec,
    op_set,
    op_add, op_sub,
    op_mul, op_div
};

struct ParsingScope{
    int parentIndex;
    int index;
};

enum ContextFlags{
    context_call = 0x1,
    context_argument = 0x2,
    context_def = 0x4
};

struct ParsingToken{
    enum ParsingTokenType TokenType;
    enum BuiltInTypes Type;
    enum ConditionalTypes Conditional;
    enum OperatorTypes Operator;
    int ScopeIndex;
    int DataIndex;
    uint16_t Context;
    char* Source;
    int Length;
};

extern char* TokenTypeStrings[];
extern char* ConditionalTypeStrings[];
extern char* BuiltInTypesString[];
extern char* OperatorTypesString[];

struct LinkedListEntry* ParseFile(struct LinkedListEntry* entry);
extern void DumpParsingLinkedEntry(struct LinkedListEntry* entry);