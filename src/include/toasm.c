#include "toasm.h"

int asmFD;
int asmIndent = 0;
DataSize globalSize = l;
enum ArchTarget globalArch = amd;

uint64_t bitMask[4] = {
    0xFF,              // byte
    0xFFFF,            // word
    0xFFFFFFFF,        // double
    0xFFFFFFFFFFFFFFFF // long
};

// amd registers and their byte equivalents
char* amdRegisterLookup[] = {
    // 16 BITS
    "ax",
    "bx",
    "cx",
    "dx",
    "di",
    "si",
    "sp",
    "bp",
    "ip",
    // 32 BITS
    "eax",
    "ebx",
    "ecx",
    "edx",
    "edi",
    "esi",
    "esp",
    "ebp",
    "eip",
    // 64 BITS
    "rax",
    "rbx",
    "rcx",
    "rdx",
    "rdi",
    "rsi",
    "rsp",
    "rbp",
    "rip"
};

char* lookupRegister(ASMRegister* reg){
    if(globalArch == amd){
        int skip = (int)globalSize;
        if(skip != 0){
            skip--;
        }
        return amdRegisterLookup[(skip*9) + (int)reg->AMD];
    }
}

// wrappers
void asm_indent(int count){
    for(int i = 0; i < count; i++){
        write(asmFD, "\t", 1);
    }
}

void asm_movrr(ASMRegister* set, ASMRegister* give){
    asm_indent(asmIndent);
    dprintf(asmFD, "mov %s, %s\n", lookupRegister(set), lookupRegister(give));
}

void asm_movri(ASMRegister* set, uint64_t give, DataSize size){
    asm_indent(asmIndent);
    uint64_t masked = give & (bitMask[(int)size]);
    dprintf(asmFD, "mov %s, %i\n", lookupRegister(set), masked);
}

void asm_movrn(ASMRegister* set, char* name, size_t nameLen, DataSize size){
    asm_indent(asmIndent);
    dprintf(asmFD, "mov %s, [%.*s]\n", lookupRegister(set), nameLen, name);
}

void asm_label(struct NameTableEntry* nte){
    asm_indent(asmIndent);
    dprintf(asmFD, "%.*s:\n", nte->nameLength, nte->name);
    asmIndent++;
}

void asm_call(struct NameTableEntry* nte){
    asm_indent(asmIndent);
    dprintf(asmFD, "call %.*s\n", nte->nameLength, nte->name);
}

void asm_push(ASMRegister* reg){
    asm_indent(asmIndent);
    dprintf(asmFD, "push %s\n", lookupRegister(reg));
}

void asm_pop(ASMRegister* reg){
    asm_indent(asmIndent);
    dprintf(asmFD, "pop %s\n", lookupRegister(reg));
}

void asm_ret(){
    asmIndent--;
    asm_indent(asmIndent);
    dprintf(asmFD, "ret");
}

void asm_retr(ASMRegister* reg){
    asm_indent(asmIndent);
    dprintf(asmFD, "push %s\nret\n", lookupRegister(reg));
}

// creation
void asmWrapperTest(char* dir){
    struct NameTableEntry nte;
    asmFD = open(dir, O_APPEND | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(asmFD == -1){ return; }
    truncate(dir, 0);

    char* name = "testLabel";
    nte.name = name;
    nte.nameLength = 9;

    asm_label(&nte);
    ASMRegister raxReg; raxReg.AMD = rax;
    ASMRegister rbxReg; rbxReg.AMD = rbx;
    //printf("%s\n", lookupRegister(&raxReg));
    asm_movrr(&raxReg, &rbxReg);

    close(asmFD);
}

DataSize typeSizes[] = {
    b, // i8
    w, // i16
    d, // i32
    l, // i64
    b, // ch
    l, // none
    l, // point
    l, // addr
    l  // arr
};

void OutputASM(char* dir, struct ParsingResult* parsed){
    // small mini ASM output meant to be used with test/src/prealpha.w
    // outputs a label and some calls with args
    asmFD = open(dir, O_APPEND | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(asmFD == -1){ return; }
    truncate(dir, 0);

    struct LinkedListEntry* tokens = parsed->TokenList;

    struct LinkedListEntry* cur;
    for(cur = tokens; cur != NULL; cur = cur->next){
        struct ParsingToken* curTok = (struct ParsingToken*)cur->self;
        if(curTok->TokenType == token_name){
            if(curTok->Context & context_def && curTok->Context & context_function){
                struct NameTableEntry* nte = DefEntryByName(parsed->NameTable, curTok->Source, curTok->Length);
                if(nte == NULL){ printf("FAILURE: couldn't find name\n"); return; };
                if(nte->Status == table_promise){ 
                    // ignore since it's defined in a separate file
                    continue;
                }
                printf("%.*s\n", nte->nameLength, nte->name);
                asm_label(nte);
            }else if(curTok->Context & context_call){
                struct NameTableEntry* nte = DefEntryByName(parsed->NameTable, curTok->Source, curTok->Length);
                if(nte == NULL){ printf("FAILURE: couldn't find name\n"); return; };
                printf("call %.*s\n", nte->nameLength, nte->name);

                // push args, no checking needed as parse.c does that
                struct LinkedListEntry* curArg;
                struct ParsingToken* curArgT;
                int seenArgs = 0;
                ASMRegister pushReg; pushReg.AMD = rdx; pushReg.ARM = x8;
                for(curArg = cur; curArg != NULL; curArg = curArg->next){
                    curArgT = (struct ParsingToken*)curArg->self;
                    if(curArgT->TokenType == token_number){
                        char tmp[128]; snprintf(tmp, 128, "%.*s", curArgT->Length, curArgT->Source);
                        char* endptr = NULL;
                        uint64_t give = strtoull(tmp, &endptr, 10);
                        DataSize size = typeSizes[(int)nte->argv[seenArgs++]->Type];
                        asm_movri(&pushReg, give, size);
                        asm_push(&pushReg);
                    }else if(curArgT->TokenType == token_name){

                    }else if(curArgT->TokenType == token_argumentEnd){
                        break;
                    }
                }

                asm_call(nte);
            }
        }else if(curTok->TokenType == token_scopeEnd){
            switch (curTok->Scope->owner->TokenType){
                case token_function:{
                    asm_ret();
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }

    close(asmFD);
}