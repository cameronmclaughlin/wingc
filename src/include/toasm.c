#include "toasm.h"

int asmFD;
int asmIndent = 0;

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
    if(globalSettings.globalArch == amd){
        int skip = (int)globalSettings.globalSize;
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

void asm_raw(char* src, size_t len){
    asm_indent(asmIndent);
    if(len == -1){ len = strlen(src); }
    dprintf(asmFD, "%.*s\n", len, src);
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

void asm_mathri(ASMRegister* store, enum OperatorTypes op, uint64_t give, DataSize size){
    asm_indent(asmIndent);
    uint64_t masked = give & (bitMask[(int)size]);
    if(op == op_add){
        dprintf(asmFD, "add %s, %i\n", lookupRegister(store), masked);
    }else if(op == op_sub){
        dprintf(asmFD, "sub %s, %i\n", lookupRegister(store), masked);
    }
}

void asm_mathrr(ASMRegister* store, ASMRegister* b, enum OperatorTypes op){
    asm_indent(asmIndent);
    if(op == op_add){
        dprintf(asmFD, "add %s, %i\n", lookupRegister(store), lookupRegister(b));
    }else if(op == op_sub){
        dprintf(asmFD, "sub %s, %i\n", lookupRegister(store), lookupRegister(b));
    }
}

void asm_movrn(ASMRegister* set, char* name, size_t nameLen){
    asm_indent(asmIndent);
    dprintf(asmFD, "mov %s, [%.*s]\n", lookupRegister(set), nameLen, name);
}

void asm_movrro(ASMRegister* set, ASMRegister* take, uint64_t offset, bool sub){
    asm_indent(asmIndent);
    if(sub){
        dprintf(asmFD, "mov %s, [%s - %i]\n", lookupRegister(set), lookupRegister(take), offset);
    }else{  
        dprintf(asmFD, "mov %s, [%s + %i]\n", lookupRegister(set), lookupRegister(take), offset);
    }
}

void asm_movror(ASMRegister* set, uint64_t offset, ASMRegister* take, bool sub){
    asm_indent(asmIndent);
    if(sub){
        dprintf(asmFD, "mov [%s - %i], %s\n", lookupRegister(set), offset, lookupRegister(take));
    }else{
        dprintf(asmFD, "mov [%s + %i], %s\n", lookupRegister(set), offset, lookupRegister(take));
    }
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
    dprintf(asmFD, "ret\n");
}

void asm_retp(int pop){
    asmIndent--;
    asm_indent(asmIndent);
    dprintf(asmFD, "ret %i\n", pop);
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

int DataSizeBytes[] = {
    1,
    2,
    4,
    8
};

void OutputASM(char* dir, struct ParsingResult* parsed){
    // small mini ASM output meant to be used with test/src/prealpha.w
    // outputs a label and some calls with args
    asmFD = open(dir, O_APPEND | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(asmFD == -1){ return; }
    truncate(dir, 0);

    // comments
    dprintf(asmFD, "; OS %s\n", OSStrings[globalSettings.globalOS]);
    dprintf(asmFD, "; ARCH %s-%s\n", ArchStrings[globalSettings.globalArch], DataSizeStrings[globalSettings.globalSize]);

    ASMRegister pushReg; pushReg.AMD = rdx; pushReg.ARM = x8;
    ASMRegister bpReg; bpReg.AMD = rbp; bpReg.ARM = x29;
    ASMRegister spReg; spReg.AMD = rsp; spReg.ARM = SP;
    ASMRegister argsReg; argsReg.AMD = rcx; argsReg.ARM = x2;

    struct LinkedListEntry* tokens = parsed->TokenList;

    int stackSep = DataSizeBytes[globalSettings.globalSize];

    struct NameTableEntry* mainNTE = DefEntryByName(parsed->NameTable, "main", 4);
    if(mainNTE == NULL){
        printf("Error! No main function\n");
        close(asmFD);
        return;
    }

    // only process text section
    /*
        for when wingas is started
        asm_raw(";text", -1);
        asm_raw("expose _entry", -1);
        asm_raw("_entry:", -1);
        asm_raw("\tcall main", -1);
    */
    // nasm
    asm_raw("section .text", -1);
    asm_raw("global _start", -1);
    asm_raw("_start:", -1);
    asm_raw("\t call main", -1);
    
    struct LinkedListEntry* cur;
    struct NameTableEntry* lastNTE;
    for(cur = tokens; cur != NULL; cur = cur->next){
        struct ParsingToken* curTok = (struct ParsingToken*)cur->self;
        if(curTok->TokenType == token_name){
            if(curTok->Context & context_def && curTok->Context & context_function){
                struct NameTableEntry* nte = DefEntryByName(parsed->NameTable, curTok->Source, curTok->Length);
                lastNTE = nte;
                if(nte == NULL){ printf("FAILURE: couldn't find name\n"); return; };
                if(nte->Status == table_promise){ 
                    // ignore since it's defined in a separate file
                    continue;
                }
                printf("%.*s w/ %i var(s), %i arg(s), and scope %i/%i\n", nte->nameLength, nte->name, nte->owner->Scope->variableDefs, nte->argc, nte->owner->Scope->index, nte->owner->Scope->parentIndex);
                asm_label(nte);
                asm_push(&bpReg);
                asm_movrr(&bpReg, &spReg);
                int bufferSize = (stackSep * nte->argc) + (stackSep * nte->owner->Scope->variableDefs);
                bufferSize+=(bufferSize % (stackSep*2) == 0) ? 0 : stackSep;
                asm_mathri(&spReg, op_sub, bufferSize, l);
                // okay now transfer args to local buffer
                int curOffset = stackSep + (stackSep * nte->argc);
                int curBuffOffset = bufferSize - stackSep;
                for(int i = 0; i < nte->argc; i++){
                    printf("ARG %i at %i\n", i, curOffset);
                    asm_movrro(&argsReg, &bpReg, curOffset, false);
                    asm_movror(&bpReg, curBuffOffset, &argsReg, true);

                    // set the args nte to it
                    struct ParsingToken* argTok = nte->argv[i];
                    struct NameTableEntry* argNTE = EntryByNameAndScope(parsed->NameTable, parsed->ScopeList, argTok->Source, argTok->Length, argTok->Scope->index);
                    argNTE->rbpOffset = curBuffOffset;
                    nte->pushedVars++;

                    curBuffOffset += stackSep;
                    curOffset -= stackSep;
                }
            }else if(curTok->Context & context_call){
                struct NameTableEntry* nte = DefEntryByName(parsed->NameTable, curTok->Source, curTok->Length);
                if(nte == NULL){ printf("FAILURE: couldn't find name\n"); return; };
                printf("call %.*s\n", nte->nameLength, nte->name);

                // push args, no checking needed as parse.c does that
                struct LinkedListEntry* curArg;
                struct ParsingToken* curArgT;
                int seenArgs = 0;
                for(curArg = cur->next; curArg != NULL; curArg = curArg->next){
                    curArgT = (struct ParsingToken*)curArg->self;
                    if(curArgT->TokenType == token_number){
                        char tmp[128]; snprintf(tmp, 128, "%.*s", curArgT->Length, curArgT->Source);
                        char* endptr = NULL;
                        uint64_t give = strtoull(tmp, &endptr, 10);
                        DataSize size = typeSizes[(int)nte->argv[seenArgs++]->Type];
                        asm_movri(&pushReg, give, size);
                        asm_push(&pushReg);
                    }else if(curArgT->TokenType == token_name){
                        struct NameTableEntry* argNTE = EntryByNameAndScope(parsed->NameTable, parsed->ScopeList, curArgT->Source, curArgT->Length, curArgT->Scope->index);
                        if(argNTE == NULL){ printf("no argNTE\n");}
                        asm_movrro(&pushReg, &bpReg, argNTE->rbpOffset, true);
                        //asm_movrn(&pushReg, curArgT->Source, curArgT->Length);
                        asm_push(&pushReg);
                    }else if(curArgT->TokenType == token_argumentEnd){
                        break;
                    }
                }

                asm_call(nte);
            }else if(curTok->Context & context_def){
                // set up the rbp offset for this
                printf("SAW VAR DEF %.*s\n", curTok->Length, curTok->Source);
                // nte for func that defines it would be lastNTE
                int topOffset = stackSep + (stackSep * lastNTE->argc);
                int offset = topOffset + (lastNTE->pushedVars * stackSep);
                lastNTE->pushedVars++;
                printf("VAR %.*s AT %i\n", curTok->Length, curTok->Source, offset);
                struct NameTableEntry* varNTE = EntryByNameAndScope(parsed->NameTable, parsed->ScopeList, curTok->Source, curTok->Length, curTok->Scope->index);
                varNTE->rbpOffset = offset;
                if(varNTE->Status == table_definition){
                    // set the value to whatever set to
                    struct ParsingToken* valTok = (struct ParsingToken*)cur->next->next->self;
                    if(curTok->Type < 3){
                        // number
                        printf("VALUE %.*s\n", valTok->Length, valTok->Source);
                        char tmp[128]; snprintf(tmp, 128, "%.*s", valTok->Length, valTok->Source);
                        char* endptr = NULL;
                        uint64_t give = strtoull(tmp, &endptr, 10);
                        DataSize size = typeSizes[curTok->Type];
                        asm_movri(&pushReg, give, size);
                        asm_movror(&bpReg, offset, &pushReg, true);
                    }
                }
            }
        }else if(curTok->TokenType == token_scopeEnd){
            struct ParsingToken* nameTok = curTok->Scope->nameOwner;
            switch (curTok->Scope->owner->TokenType){
                case token_function:{
                    printf("find %.*s\n", nameTok->Length, nameTok->Source);
                    struct NameTableEntry* nte = DefEntryByName(parsed->NameTable, nameTok->Source, nameTok->Length);
                    int bufferSize = (stackSep * nte->argc) + (stackSep * nte->owner->Scope->variableDefs);
                    bufferSize+=(bufferSize % (stackSep*2) == 0) ? 0 : stackSep;
                    asm_mathri(&spReg, op_add, bufferSize, l);
                    asm_pop(&bpReg);
                    if(nte->argc == 0){
                        asm_ret();
                    }else{
                        asm_retp(stackSep * nte->argc);
                    }
                    break;
                }
                default:{
                    break;
                }
            }
        }else if(curTok->TokenType == token_asm){
            // assembly insert
            struct ParsingToken* asmBody = (struct ParsingToken*)cur->next->self;
            asm_raw(asmBody->Source, asmBody->Length);
            printf("ASSEMBLY %.*s\n", asmBody->Length, asmBody->Source);
        }else if(curTok->TokenType == token_number){
            if(curTok->Context & context_argument){
                // is a num argument. just push
                char tmp[128]; snprintf(tmp, 128, "%.*s", curTok->Length, curTok->Source);
                char* endptr = NULL;
                uint64_t give = strtoull(tmp, &endptr, 10);
                DataSize size = typeSizes[curTok->Type];
                asm_movri(&pushReg, give, size);
                asm_push(&pushReg);
            }
        }
    }

    close(asmFD);
}