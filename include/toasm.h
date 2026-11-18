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
#include "lexer.h"
#include "linked.h"
#include "nametable.h"

enum ArchTarget{
    amd,
    arm
};

enum AMDRegisters{
    rax, // used for math (1st input, store)
    rbx, // used for math (2nd input)
    rcx, // used for for loops
    rdx,
    rdi,
    rsi,
    rsp,
    rbp,
    rip
};

enum ARMRegisters{
    x0, x1, x2, x3,
    x4, x5, x6, x7,
    x8, x9, x10, x11,
    x12, x13, x14, x15,
    x16, x17, x18, x19,
    x20, x21, x22, x23,
    x24, x25, x26, x27,
    x28, x29, x30,
    SP,
    PC,
    PSTATE
};

typedef struct ASMRegister{
    enum AMDRegisters AMD;
    enum ARMRegisters ARM;
} ASMRegister;

typedef enum DataSize{
    b, // 8 bits
    w, // 16 bits
    d, // 32 bits
    l  // 64 bits
} DataSize;

// wrappers
void asm_movrr(ASMRegister* set, ASMRegister* give);
void asm_movri(ASMRegister* set, uint64_t give, DataSize size);
void asm_label(struct NameTableEntry* nte);
void asm_push(ASMRegister* reg);
void asm_pop(ASMRegister* reg);

// creation
void asmWrapperTest(char* dir);
void OutputASM(char* dir, struct ParsingResult* parsed);