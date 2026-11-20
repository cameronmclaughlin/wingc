#pragma once
#include <stdbool.h>
#include <stdint.h>

#define IMPLEMENTED_TOKENS 13
#define IMPLEMENTED_CONDITIONALS 6
#define IMPLEMENTED_TYPES 9
#define IMPLEMENTED_OPERATORS 6
#define IMPLEMENTED_PREPROCESSORS 6

enum ParsingTokenType{
    token_scopeStart,
    token_scopeEnd,
    token_argumentStart,
    token_argumentEnd,
    token_function,
    token_while,
    token_for,
    token_if,
    token_preprocessor,
    token_set,
    token_asm,
    token_separator,
    token_end,
    token_type,
    token_conditional,
    token_number,
    token_operator,
    token_invalid,
    token_filestart,
    token_name,
    token_string
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
    struct ParsingToken* nameOwner;
    int variableDefs;
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
    context_function = 0x10,
    context_global = 0x11
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
    table_promise,
    table_reference
};

enum NameTableType{
    tabledef_function,
    tabledef_variable
};

typedef enum Preprocessors{
    prep_none,
    prep_bytes,
    prep_os,
    prep_cpu,
    prep_endif,
    prep_ifbits,
    prep_ifos
} Preprocessors;

struct NameTableEntry{
    char* name;
    size_t nameLength;
    int argc; // MAX 32
    struct ParsingToken* argv[32];
    struct ParsingToken* owner;
    enum ParsingTokenType ReturnType;
    enum NameTableStatus Status;
    enum NameTableType Type;
    int rbpOffset;
    int pushedVars;
};

typedef enum DataSize{
    b, // 8 bits
    w, // 16 bits
    d, // 32 bits
    l  // 64 bits
} DataSize;

enum ArchTarget{
    amd,
    arm
};

enum OSTarget{
    os_lnx,
    os_bsd,
    os_mac,
    os_win,
    os_bios,
    os_uefi
};

typedef struct CompilerSettings{
    DataSize globalSize;
    enum OSTarget globalOS;
    enum ArchTarget globalArch;
} CompilerSettings;

extern CompilerSettings globalSettings;

extern char* TokenTypeStrings[];
extern char* ConditionalTypeStrings[];
extern char* BuiltInTypesString[];
extern char* OperatorTypesString[];

// config strings
extern char* OSStrings[];
extern char* ArchStrings[];
extern char* DataSizeStrings[];

struct TokenBinaryTree{
    enum ParsingTokenType* type;
    struct LinkedListEntry* tokenList;
    size_t tokenListLen;
    struct BinaryTree* left;
    struct BinaryTree* right;
    struct BinaryTree* top;
};

extern bool failureFlag;