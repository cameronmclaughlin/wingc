#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#define IMPLEMENTED_TOKENS 14
#define IMPLEMENTED_CONDITIONALS 6
#define IMPLEMENTED_TYPES 9

enum ParsingTokenType{
    token_invalid,
    token_name,
    token_scopeStart,
    token_scopeEnd,
    token_argumentStart,
    token_argumentEnd,
    token_function,
    token_set,
    token_while,
    token_for,
    token_preprocessor,
    token_type,
    token_conditional,
    token_end,
    token_number
};

enum ConditionalTypes{
    cond_equals,
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

struct ParsingScope{
    int ID;
    int parentID;
};

struct ParsingToken{
    enum ParsingTokenType TokenType;
    enum BuiltInTypes Type;
    enum ConditionalTypes Conditional;
    struct ParsingScope Scope;
    int DataIndex;
    char* Source;
    int Length;
};

struct ParsingLinkedEntry{
    struct ParsingLinkedEntry* last;
    struct ParsingLinkedEntry* next;
    struct ParsingToken* token;
};

struct ParsingLinkedEntry* ParseFile(char* dir);