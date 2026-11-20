// WINGC COPYRIGHT Cameron McLaughlin 2026
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parse.h"
#include "linked.h"
#include "toasm.h"

struct Arguments{
    int outputPos;
    int inputPos;
};

enum argFlags{
    output,
    input
};

char* argFlagStrings[] = {"-o", "-i"};

void matchArgs(char *str, int index, struct Arguments *args){
    for(int i = 0; i < 2; i++){
        if(strcmp(str, argFlagStrings[i]) == 0){
            switch((enum argFlags)i){
                case output:{
                    args->outputPos = index+1;
                    break;
                }
                case input:{
                    args->inputPos = index+1;
                    break;
                }
            }
        }
    }
}

int main(int argc, char* argv[]){
    // scan args for inputs and outputs
    struct Arguments args;
    memset((void*)&args, 0, sizeof(args));

    for(int i = 1; i < argc; i++){
        matchArgs(argv[i], i, &args);
    }
    printf("WingC %s\n", WINGC_VERSION);
    
    if(args.inputPos == 0){ printf("No input given, use -i <source file>"); return -1; }
    printf("Compiling %s to ", argv[args.inputPos]); if(args.outputPos != 0){ printf(" -> %s", argv[args.outputPos]); } printf("\n");

    if(access(argv[args.inputPos], F_OK) != 0){
        // input non existent
        printf("ERROR: Input file %s nonexistent\n", argv[args.inputPos]);
        return -1;
    }

    struct LinkedListEntry* headEntry = LexizeFile(argv[args.inputPos]);
    if(failureFlag == true){
        FreeLinkedList(headEntry);
        return -1;
    }
    // now that lexing is done. time to parse
    printf("PARSING\n");
    struct ParsingResult* parsed = ParseFile(headEntry);
    if(failureFlag == true){
        free(((struct ParsingToken*)parsed->TokenList->self)->Source);

        FreeLinkedList(parsed->NameTable);
        FreeLinkedList(parsed->TokenList);
        FreeLinkedList(parsed->ScopeList);
        return -1;
    }
    OutputASM(argv[args.outputPos], parsed);
    //printf("%s", argv[args.outputPos]);

    free(((struct ParsingToken*)parsed->TokenList->self)->Source);

    FreeLinkedList(parsed->NameTable);
    FreeLinkedList(parsed->TokenList);
    FreeLinkedList(parsed->ScopeList);

    //asmWrapperTest("test/bin/test.a");

    return 0;
}