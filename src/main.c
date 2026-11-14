// WING COPYRIGHT Cameron McLaughlin 2026
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"

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
    printf("Compiling %s", argv[args.inputPos]); if(args.outputPos != 0){ printf(" -> %s", argv[args.outputPos]); } printf("\n");

    ParseFile(argv[args.inputPos]);

    return 0;
}