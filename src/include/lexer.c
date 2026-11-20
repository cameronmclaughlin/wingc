#include "lexer.h"

CompilerSettings globalSettings;

char* TokenTypeStrings[] = {
    "{", "}",
    "(", ")",
    "fn",
    "while",
    "for",
    "if",
    "!",
    "=",
    "_asm",
    ",",
    ";",
    "TYPE", "COND", 
    "NUMBER", "OPERATOR",
    "INVALID", "FILE_START", "NAME",
    "STRING"
};

char* ConditionalTypeStrings[] = {
    "==", "~=",
    "<", ">",
    "<=", ">="
};

char* BuiltInTypesString[] = {
    "i8", "i16", "i32", "i64",
    "ch", 
    "none", 
    "point", "addr",
    "arr"
};

char* OperatorTypesString[] = {
    "+=", "-=",
    "+", "-",
    "*", "/"
};

bool tokenMatches(char* a, char* b){
    return strncmp(a, b, strlen(b)) == 0;
}

void DumpParsingLinkedEntry(struct LinkedListEntry* entry){
    struct ParsingToken* tok = (struct ParsingToken*)entry->self;
    int i = tok->DataIndex;
    if(tok->TokenType == token_type){
        printf("TYPE %s (%i) at %i\n", BuiltInTypesString[tok->Type], tok->Type, i);
    }else if(tok->TokenType == token_conditional){
        printf("CONDITIONAL %s (%i) at %i\n", ConditionalTypeStrings[tok->Conditional], tok->Conditional, i);
    }else if(tok->TokenType == token_operator){
        printf("OPERATOR %s (%i) at %i\n", OperatorTypesString[tok->Operator], tok->Operator, i);
    }else{
        printf("KEYWORD %s (%i) at %i\n", TokenTypeStrings[tok->TokenType], tok->TokenType, i);
    }
}

char* PrepStrings[] = {
    "bits",
    "os",
    "arch",
    "endif",
    "ifbits",
    "ifos"
};

char* OSStrings[] = {
    "LINUX",
    "BSD",
    "MAC",
    "WINDOWS",
    "BIOS",
    "UEFI"
};

char* ArchStrings[] = {
    "AMD",
    "ARM"
};
char* DataSizeStrings[] = {
    "8",
    "16",
    "32",
    "64"
};

enum ParsingTokenType* PrepArgT[] = {
    (enum ParsingTokenType[]){token_number}, // bits
    (enum ParsingTokenType[]){token_name}, // os
    (enum ParsingTokenType[]){token_name}, // arch
    NULL,
    (enum ParsingTokenType[]){token_number} // ifbits
};

size_t PrepArgC[] = {
    1, // bits
    1, // os
    1,  // arch
    0, // endif
    1 // ifbits
};

struct LinkedListEntry *cur, *next, *temp, *swap;
int tokIndex, tempIndex;

bool prepIfTrue = false;

void Preprocess(Preprocessors prepType, int argc, struct ParsingToken* argv){
    printf("PREPROCESS\n");
    if(prepType == prep_bytes){
        struct ParsingToken* target = &argv[0];
        if(tokenMatches(target->Source, "64")){
            globalSettings.globalSize = l;
            return;
        }else if(tokenMatches(target->Source, "32")){
            globalSettings.globalSize = d;
            return;
        }else if(tokenMatches(target->Source, "16")){
            globalSettings.globalSize = w;
            return;
        }else{
            printf("Unrecognized byte target %.*s\n", target->Length, target->Source);
            return;
        }
    }else if(prepType == prep_os){
        struct ParsingToken* target = &argv[0];
        if(tokenMatches(target->Source, "lnx") || tokenMatches(target->Source, "linux")){
            globalSettings.globalOS = os_lnx;
            return;
        }else if(tokenMatches(target->Source, "bsd") || tokenMatches(target->Source, "freebsd")){
            globalSettings.globalOS = os_bsd;
            return;
        }else if(tokenMatches(target->Source, "mac") || tokenMatches(target->Source, "macos")){
            globalSettings.globalOS = os_mac;
            return;
        }else if(tokenMatches(target->Source, "win") || tokenMatches(target->Source, "windows")){
            globalSettings.globalOS = os_win;
            return;
        }else if(tokenMatches(target->Source, "bios")){
            globalSettings.globalOS = os_bios;
            return;
        }else if(tokenMatches(target->Source, "uefi")){
            globalSettings.globalOS = os_uefi;
            return;
        }else{
            printf("Unrecognized OS target %.*s\n", target->Length, target->Source);
            return;
        }
    }else if(prepType == prep_cpu){
        struct ParsingToken* target = &argv[0];
        if(tokenMatches(target->Source, "amd")){
            globalSettings.globalArch = amd;
            return;
        }else if(tokenMatches(target->Source, "arm")){
            globalSettings.globalArch = arm;
            return;
        }else{
            printf("Unrecognized CPU architecture %.*s\n", target->Length, target->Source);
            return;
        }
    }else if(prepType == prep_ifbits){
        struct ParsingToken* target = &argv[0];
        temp = cur;
        cur = swap;
        
        if(tokenMatches(target->Source, "64") && globalSettings.globalSize == l){
            prepIfTrue = true;
        }else if(tokenMatches(target->Source, "32") && globalSettings.globalSize == d){
            prepIfTrue = true;
        }else if(tokenMatches(target->Source, "16") && globalSettings.globalSize == w){
            prepIfTrue = true;
        }else{
            prepIfTrue = false;
        }

        tempIndex = tokIndex;
        tokIndex = 0;
    }else if(prepType == prep_ifos){
        struct ParsingToken* target = &argv[0];

        temp = cur;
        cur = swap;

        if((tokenMatches(target->Source, "lnx") || tokenMatches(target->Source, "linux")) && globalSettings.globalOS == os_lnx){
            prepIfTrue = true;
        }else if((tokenMatches(target->Source, "bsd") || tokenMatches(target->Source, "freebsd")) && globalSettings.globalOS == os_bsd){
            prepIfTrue = true;
        }else if((tokenMatches(target->Source, "mac") || tokenMatches(target->Source, "macos")) && globalSettings.globalOS == os_mac){
            prepIfTrue = true;
        }else if((tokenMatches(target->Source, "win") || tokenMatches(target->Source, "windows")) && globalSettings.globalOS == os_win){
            prepIfTrue = true;
        }else if(tokenMatches(target->Source, "bios") && globalSettings.globalOS == os_bios){
            prepIfTrue = true;
        }else if(tokenMatches(target->Source, "uefi") && globalSettings.globalOS == os_uefi){
            prepIfTrue = true;
        }else{
            prepIfTrue = false;
        }

        tempIndex = tokIndex;
        tokIndex = 0;
    }else if(prepType == prep_endif){
        struct ParsingToken* target = &argv[0];
        struct LinkedListEntry* swapTail = cur;
        cur = temp;
        tokIndex = tempIndex;
        if(prepIfTrue){
            cur->next = swap;
            cur = swapTail;
            tokIndex++;
            //printf("TRUE\n");
        }else{
            //printf("FALSE\n");
            FreeLinkedList(swap);
        }
        swap = CreateLinkedListEntry(sizeof(struct ParsingToken));
    }
}

bool failureFlag = false;

struct LinkedListEntry* LexizeFile(char* dir){
    globalSettings.globalOS = os_lnx;
    globalSettings.globalArch = amd;
    globalSettings.globalSize = l;

    struct stat f_stats;
    stat(dir, &f_stats);
    char* buffer = malloc(f_stats.st_size + 1);
    memset(buffer, 0, f_stats.st_size+1);
    
    int fd = open(dir, O_RDONLY);
    read(fd, buffer, f_stats.st_size);

    int lineCount = 0;
    for(int i = 0; i < f_stats.st_size; i++){
        if(buffer[i] == '\0' || buffer[i] == '\n'){ lineCount++; }
    }

    struct LinkedListEntry* headEntry = CreateLinkedListEntry(sizeof(struct ParsingToken));
    if(headEntry == NULL){ printf("NO HEAD\n"); return NULL; }

    char* src = buffer;
    ((struct ParsingToken*)headEntry->self)->TokenType = token_filestart;
    ((struct ParsingToken*)headEntry->self)->Source = buffer;

    tokIndex = 0;

    cur = headEntry;
    swap = CreateLinkedListEntry(sizeof(struct ParsingToken));

    bool inPreprocessor = false;
    Preprocessors prepType = prep_none;
    int prepArgc = 0;
    struct ParsingToken prepArgv[16]; memset(prepArgv, 0, sizeof(struct ParsingToken) * 16);

    while(*src){
        if(isspace(*src)){ src++; continue; }

        if(inPreprocessor){
            // scan for preprocessing
            if(*src == '!'){
                // dump
                //printf("PREPROCESSOR %s\n", PrepStrings[prepType-1]);
                for(int i = 0; i < prepArgc; i++){
                    struct ParsingToken* cur = &prepArgv[i];
                    if(cur->TokenType == token_name){
                        //printf("PREP NAME ARG %.*s\n", cur->Length, cur->Source);
                    }else if(cur->TokenType == token_number){
                        //printf("PREP NUMBER ARG %.*s\n", cur->Length, cur->Source);
                    }
                }
                inPreprocessor = false;
                Preprocess(prepType, prepArgc, prepArgv);
                prepType = prep_none;
                src++;
                continue;
            }

            if(prepType == prep_none){
                //printf("searching\n");
                // fetch type
                for(int i = 0; i < IMPLEMENTED_PREPROCESSORS; i++){
                    if(tokenMatches(src, PrepStrings[i])){
                        prepType = (Preprocessors)i+1;
                        src+=strlen(PrepStrings[i]);
                        //printf("Found preprocessor %s\n", PrepStrings[i]);
                        continue;
                    }
                }
                if(prepType == prep_none){
                    //printf("Unrecognized preprocessor\n");
                    failureFlag = true;
                    return headEntry;
                }
            }else{
                if(!(prepArgc < PrepArgC[(int)prepType-1])){
                    /*
                    printf(
                        "ERROR: too many preprocessor arguments. Expected %i but got %i\n",
                        PrepArgC[(int)prepType-1],
                        prepArgc+1
                    );
                    */
                    failureFlag = true;
                    return headEntry;
                }

                // okay args now
                if(isalpha(*src) || *src == '_'){
                    // name arg
                    if(PrepArgT[(int)prepType-1][prepArgc] != token_name){ 
                        /*
                        printf(
                            "ERROR: Invalid preprocessor argument for %s. Expected %s but got %s\n",
                            PrepStrings[(int)prepType-1],
                            TokenTypeStrings[PrepArgT[(int)prepType-1][prepArgc]],
                            TokenTypeStrings[(int)token_name]
                        );
                        */
                        failureFlag = true;
                        return headEntry;
                    }
                    struct ParsingToken* newTok = &prepArgv[prepArgc];
                    newTok->Source = src;
                    size_t len = 0;
                    while(isalnum(*src) || *src == '_'){
                        src++;
                        len++;
                    }
                    newTok->Length = len;
                    newTok->TokenType = token_name;
                    prepArgc++;
                    continue;
                }else if(isdigit(*src)){
                    // number arg
                    if(PrepArgT[(int)prepType-1][prepArgc] != token_number){ 
                        /*
                        printf(
                            "ERROR: Invalid preprocessor argument for %s. Expected %s but got %s\n",
                            PrepStrings[(int)prepType-1],
                            TokenTypeStrings[PrepArgT[(int)prepType-1][prepArgc]],
                            TokenTypeStrings[(int)token_name]
                        );
                        */
                        failureFlag = true;
                        return headEntry;
                    }
                    struct ParsingToken* newTok = &prepArgv[prepArgc];
                    newTok->Source = src;
                    size_t len = 0;
                    while(isdigit(*src)){
                        src++;
                        len++;
                    }
                    newTok->Length = len;
                    newTok->TokenType = token_number;
                    prepArgc++;
                    continue;
                }
            }
            continue;
        }

        // scan for keywords
        for(int i = 0; i < IMPLEMENTED_TOKENS; i++){
            if(tokenMatches(src, TokenTypeStrings[i])){
                if(i == token_preprocessor){
                    inPreprocessor = true;
                    src++;
                    prepArgc = 0;
                    goto skipscan;
                }
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next; next->last = cur;
                struct ParsingToken* nextTok = ((struct ParsingToken*)next->self);
                nextTok->TokenType = (enum ParsingTokenType)i;
                nextTok->Source = src;
                nextTok->Length = strlen(TokenTypeStrings[i]);
                src+=strlen(TokenTypeStrings[i]);
                cur = next;
                //printf("KEYWORD %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, tokIndex);
                tokIndex++;
                goto skipscan;
            }
        }
        // scan for conditionals
        for(int i = 0; i < IMPLEMENTED_CONDITIONALS; i++){
            if(tokenMatches(src, ConditionalTypeStrings[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next; next->last = cur;
                ((struct ParsingToken*)next->self)->TokenType = token_conditional;
                ((struct ParsingToken*)next->self)->Conditional = (enum ConditionalTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(ConditionalTypeStrings[i]);
                src+=strlen(ConditionalTypeStrings[i]);
                cur = next;
                //printf("CONDITIONAL %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, tokIndex);
                tokIndex++;
                goto skipscan;
            }
        }
        // scan for types
        for(int i = 0; i < IMPLEMENTED_TYPES; i++){
            if(tokenMatches(src, BuiltInTypesString[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next; next->last = cur;
                ((struct ParsingToken*)next->self)->TokenType = token_type;
                ((struct ParsingToken*)next->self)->Type = (enum BuiltInTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(BuiltInTypesString[i]);
                src+=strlen(BuiltInTypesString[i]);
                cur = next;
                //printf("TYPE %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, tokIndex);
                tokIndex++;
                goto skipscan;
            }
        }
        // scan for operators
        for(int i = 0; i < IMPLEMENTED_OPERATORS; i++){
            if(tokenMatches(src, OperatorTypesString[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next; next->last = cur;
                ((struct ParsingToken*)next->self)->TokenType = token_operator;
                ((struct ParsingToken*)next->self)->Type = (enum BuiltInTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(OperatorTypesString[i]);
                src+=strlen(OperatorTypesString[i]);
                cur = next;
                //printf("OPERATOR %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, tokIndex);
                tokIndex++;
                goto skipscan;
            }
        }

        if(*src == '"'){
            next = CreateLinkedListEntry(sizeof(struct ParsingToken));
            cur->next = next; next->last = cur;
            int len = 0;

            // start string scan
            src++;
            char* start = src;
            while(*src != '"'){
                len++;
                src++;
            }
            ((struct ParsingToken*)next->self)->TokenType = token_string;
            ((struct ParsingToken*)next->self)->Length = len; 
            ((struct ParsingToken*)next->self)->Source = start; 

            cur = next;
            src++;

            //printf("STRING %.*s (%i)\n", len, start, tokIndex);
            tokIndex++;
            goto skipscan;
        }

        // scan for names
        if(isalpha(*src) || *src == '_'){
            next = CreateLinkedListEntry(sizeof(struct ParsingToken));
            cur->next = next; next->last = cur;
            int len = 0;
            char* s = src;
            ((struct ParsingToken*)next->self)->Source = src;
            while(isalnum(*src) || *src == '_'){
                src++;
                len++;
            }

            ((struct ParsingToken*)next->self)->TokenType = token_name;
            ((struct ParsingToken*)next->self)->Length = len; 

            cur = next;

            //printf("NAME %.*s (%i)\n", len, s, tokIndex);
            tokIndex++;
            goto skipscan;
        }

        // scan for numbers
        if(isdigit(*src)){
            next = CreateLinkedListEntry(sizeof(struct ParsingToken));
            cur->next = next; next->last = cur;
            int len = 0;
            ((struct ParsingToken*)next->self)->Source = src;
            while(isdigit(*src)){
                src++;
                len++;
            }

            ((struct ParsingToken*)next->self)->TokenType = token_number;
            ((struct ParsingToken*)next->self)->Length = len;

            //printf("NUMBER %.*s (%i)\n", len, ((struct ParsingToken*)next->self)->Source, tokIndex);
            cur = next;
            tokIndex++;
            goto skipscan;
        }
        skipscan:;
    }

    return headEntry;
}