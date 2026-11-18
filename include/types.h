#pragma once
#include <stdbool.h>
#include <stdint.h>

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
    struct ParsingToken* owner;
};

struct ParsingResult{
    struct LinkedListEntry* NameTable;
    struct LinkedListEntry* TokenList;
    struct LinkedListEntry* ScopeList;
};

enum ContextFlags{
    context_call = 0x1,
    context_argument = 0x2,
    context_def = 0x4,
    context_return_type = 0x8,
    context_function = 0x10
};

struct ParsingToken{
    enum ParsingTokenType TokenType;
    enum BuiltInTypes Type;
    enum ConditionalTypes Conditional;
    enum OperatorTypes Operator;
    struct ParsingScope* Scope;
    int DataIndex;
    uint16_t Context;
    char* Source;
    int Length;
    bool seenFPEMDAS;
};

enum NameTableStatus{
    table_definition,
    table_promised,
    table_reference
};

enum NameTableType{
    tabledef_function,
    tabledef_variable
};

struct NameTableEntry{
    char* name;
    size_t nameLength;
    int argc; // MAX 32
    struct ParsingToken* argv[32];
    enum ParsingTokenType ReturnType;
    enum NameTableStatus Status;
    enum NameTableType Type;
};

extern char* TokenTypeStrings[];
extern char* ConditionalTypeStrings[];
extern char* BuiltInTypesString[];
extern char* OperatorTypesString[];

struct TokenBinaryTree{
    enum ParsingTokenType* type;
    struct LinkedListEntry* tokenList;
    size_t tokenListLen;
    struct BinaryTree* left;
    struct BinaryTree* right;
    struct BinaryTree* top;
};