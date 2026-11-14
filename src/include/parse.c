#include "parse.h"

struct ParsingLinkedEntry* AllocParsingLinkedEntry(){
    struct ParsingLinkedEntry* ret = (struct ParsingLinkedEntry*)malloc(sizeof(struct ParsingLinkedEntry));
    memset(ret, 0, sizeof(struct ParsingLinkedEntry));

    struct ParsingToken* token = (struct ParsingToken*)malloc(sizeof(struct ParsingToken));
    memset(token, 0, sizeof(struct ParsingToken));

    ret->token = token;

    return ret;
}

struct ParsingLinkedEntry* FetchParsingLinkedOffset(struct ParsingLinkedEntry* list, int index){
    struct ParsingLinkedEntry* cur = list;
    int i = 0;
    for(struct ParsingLinkedEntry* cur = list; cur != NULL && i < index; cur = cur->next, i++){}
    return cur;
}

void FreeParsingLinkedEntry(struct ParsingLinkedEntry* list, int index){
    struct ParsingLinkedEntry* entry = FetchParsingLinkedOffset(list, index);
    free(entry->token);
    free(entry);
}

void FreeParsingLinkedList(struct ParsingLinkedEntry* list){
    int listLength = 1;
    struct ParsingLinkedEntry* cur = list;
    while(true){
        if(cur->next == NULL){ break; }else{
            listLength++;
        }
    }

    for(int i = listLength; i > 0; i++){
        FreeParsingLinkedEntry(cur, 0);
        cur = cur->last;
    }
}

char* TokenTypeStrings[] = {
    "INVALID", "NAME",
    "{", "}",
    "(", ")",
    "fn",
    "=",
    "while",
    "for",
    "!",
    ";"
};

char* ConditionalTypeStrings[] = {
    "==", "!=",
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

bool tokenMatches(char* a, char* b){
    return strncmp(a, b, strlen(b)) == 0;
}

char* whitespaceString = "\t\n \r\v\f\0";

bool stringInArrayByLength(char* haystack[], int haystackLength, char* needle){
    for(int i = 0; i < haystackLength; i++){
        if(tokenMatches(needle, haystack[i])){ return true; } 
    }
    return false;
}

int getNextNonWhitespace(char* data, int start){
    int i = start;
    int max = strlen(data);
    if(start > max || start < 0){ return -1; }
    //while(strchr(whitespaceString, data[i++] != NULL)){}
    while(i < max && isspace(data[i++])){}
    if(i == max){ return -1; }
    return i-1;
}

int getNextWhitespace(char* data, int start){
    int i = start;
    int max = strlen(data);
    if(start > max || start < 0){ return -1; }
    while(i < max && !isspace(data[i++])){}
    if(i == max){ return -1; }
    return i-1;
}

int matchToken(char* data, int start, int* scope, struct ParsingLinkedEntry* entry){
    entry->token->DataIndex = start;
    //printf("START = %c\n", data[start]);
    // check if it is a keyword
    for(int i = 0; i < IMPLEMENTED_TOKENS; i++){
        if(tokenMatches(&data[start], TokenTypeStrings[i+2])){
            entry->token->TokenType = (enum ParsingTokenType)i+2;
            return getNextNonWhitespace(data, getNextWhitespace(data, start));
        }
    }

    // check if it is a conditional
    for(int i = 0; i < IMPLEMENTED_CONDITIONALS; i++){
        if(tokenMatches(&data[start], ConditionalTypeStrings[i])){
            entry->token->TokenType = token_conditional;
            entry->token->Conditional = (enum ConditionalTypes)i;
            return getNextNonWhitespace(data, start + strlen(ConditionalTypeStrings[i]));
        }
    }

    // check if it is a type
    for(int i = 0; i < IMPLEMENTED_TYPES; i++){
        if(tokenMatches(&data[start], BuiltInTypesString[i])){
            entry->token->TokenType = token_type;
            entry->token->Type = (enum BuiltInTypes)i;
            return getNextNonWhitespace(data, start + strlen(BuiltInTypesString[i]));
        }
    }

    entry->token->TokenType = token_name;

    return getNextNonWhitespace(data, getNextWhitespace(data, start));
}

void DumpParsingLinkedEntry(struct ParsingLinkedEntry* entry){
    int i = entry->token->DataIndex;
    if(entry->token->TokenType == token_type){
        printf("TYPE %s (%i) at %i\n", BuiltInTypesString[entry->token->Type], entry->token->Type, i);
    }else if(entry->token->TokenType == token_conditional){
        printf("CONDITIONAL %s (%i) at %i\n", ConditionalTypeStrings[entry->token->Conditional], entry->token->Conditional, i);
    }else{
        printf("KEYWORD %s (%i) at %i\n", TokenTypeStrings[entry->token->TokenType], entry->token->TokenType, i);
    }
}

bool checkTokenExpection(enum ParsingTokenType token){

}

struct ParsingLinkedEntry* ParseFile(char* dir){
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

    struct ParsingLinkedEntry* headEntry = AllocParsingLinkedEntry();
    if(headEntry == NULL){ printf("NO HEAD\n"); return NULL; }

    struct ParsingLinkedEntry* cur = headEntry;

    int lastMatch = 0;
    int curScope = 0;

    char* src = buffer;

    struct ParsingLinkedEntry* next;

    while(*src){
        if(isspace(*src)){ src++; continue; }

        // scan for keywords
        for(int i = 0; i < IMPLEMENTED_TOKENS; i++){
            if(tokenMatches(src, TokenTypeStrings[i+2])){
                next = AllocParsingLinkedEntry();
                cur->next = next;
                next->token->TokenType = (enum ParsingTokenType)i+2;
                next->token->Source = src;
                next->token->Length = strlen(TokenTypeStrings[i+2]);
                src+=strlen(TokenTypeStrings[i+2]);
                printf("KEYWORD %.*s\n", next->token->Length, next->token->Source);
                continue;
            }
        }
        // scan for conditionals
        for(int i = 0; i < IMPLEMENTED_CONDITIONALS; i++){
            if(tokenMatches(src, ConditionalTypeStrings[i])){
                next = AllocParsingLinkedEntry();
                cur->next = next;
                next->token->TokenType = token_conditional;
                next->token->Conditional = (enum ConditionalTypes)i;
                next->token->Source = src;
                next->token->Length = strlen(ConditionalTypeStrings[i]);
                src+=strlen(ConditionalTypeStrings[i]);
                printf("CONDITIONAL %.*s\n", next->token->Length, next->token->Source);
                continue;
            }
        }
        // scan for types
        for(int i = 0; i < IMPLEMENTED_TYPES; i++){
            if(tokenMatches(src, BuiltInTypesString[i])){
                next = AllocParsingLinkedEntry();
                cur->next = next;
                next->token->TokenType = token_type;
                next->token->Type = (enum BuiltInTypes)i;
                next->token->Source = src;
                next->token->Length = strlen(BuiltInTypesString[i]);
                src+=strlen(BuiltInTypesString[i]);
                printf("TYPE %.*s\n", next->token->Length, next->token->Source);
                continue;
            }
        }

        // scan for names
        if(isalpha(*src) || *src == '_'){
            next = AllocParsingLinkedEntry();
            cur->next = next;
            int len = 0;
            char* s = src;
            next->token->Source = src;
            while(isalnum(*src) || *src == '_'){
                src++;
                len++;
            }

            next->token->TokenType = token_name;
            next->token->Length = len; 

            cur = next;

            printf("NAME %.*s\n", len, s);
            continue;
        }

        // scan for numbers
        if(isdigit(*src)){
            next = AllocParsingLinkedEntry();
            cur->next = next;
            int len = 0;
            next->token->Source = src;
            while(isdigit(*src)){
                src++;
                len++;
            }

            next->token->TokenType = token_number;
            next->token->Length = len;

            printf("NUMBER %.*s\n", len, next->token->Source);
            cur = next;
            continue;
        }
    }

    // now that lexing is done. time to parse

    return headEntry;

    while(lastMatch != -1){
        lastMatch = matchToken(buffer, lastMatch, &curScope, cur);
        DumpParsingLinkedEntry(cur);
        //printf("Last = %i\n", lastMatch);
        cur->next = AllocParsingLinkedEntry();
        cur = cur->next;
    }

    return headEntry;
}