#include "lexer.h"

struct ParsingLinkedEntry* AllocParsingLinkedEntry(){
    struct ParsingLinkedEntry* ret = (struct ParsingLinkedEntry*)malloc(sizeof(struct ParsingLinkedEntry));
    memset((void*)ret, 0, sizeof(struct ParsingLinkedEntry));

    struct ParsingToken* token = (struct ParsingToken*)malloc(sizeof(struct ParsingToken));
    memset((void*)token, 0, sizeof(struct ParsingToken));

    ret->token = token;

    return ret;
}

struct ParsingLinkedEntry* FetchParsingLinkedOffset(struct ParsingLinkedEntry* list, int index){
    struct ParsingLinkedEntry* cur = list;
    for(int i = 0; cur != NULL && i < index; cur = cur->next, i++){}
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
    "{", "}",
    "(", ")",
    "fn",
    "while",
    "for",
    "!",
    "=",
    ";",
    "TYPE", "COND", 
    "NUMBER", "OPERATOR",
    "INVALID", "FILE_START", "NAME"
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

char* OperatorTypesString[] = {
    "+=", "-=",
    "+", "-",
    "*", "/"
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
        if(tokenMatches(&data[start], TokenTypeStrings[i])){
            entry->token->TokenType = (enum ParsingTokenType)i;
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

struct LinkedListEntry* LexizeFile(char* dir){
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

    int lastMatch = 0;

    char* src = buffer;
    ((struct ParsingToken*)headEntry->self)->TokenType = token_filestart;

    int index = 0;
    struct LinkedListEntry *cur, *next; cur = headEntry;

    while(*src){
        if(isspace(*src)){ src++; continue; }

        // scan for keywords
        for(int i = 0; i < IMPLEMENTED_TOKENS; i++){
            if(tokenMatches(src, TokenTypeStrings[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next;
                ((struct ParsingToken*)next->self)->TokenType = (enum ParsingTokenType)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(TokenTypeStrings[i]);
                src+=strlen(TokenTypeStrings[i]);
                cur = next;
                printf("KEYWORD %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, index);
                index++;
                continue;
            }
        }
        // scan for conditionals
        for(int i = 0; i < IMPLEMENTED_CONDITIONALS; i++){
            if(tokenMatches(src, ConditionalTypeStrings[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next;
                ((struct ParsingToken*)next->self)->TokenType = token_conditional;
                ((struct ParsingToken*)next->self)->Conditional = (enum ConditionalTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(ConditionalTypeStrings[i]);
                src+=strlen(ConditionalTypeStrings[i]);
                cur = next;
                printf("CONDITIONAL %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, index);
                index++;
                continue;
            }
        }
        // scan for types
        for(int i = 0; i < IMPLEMENTED_TYPES; i++){
            if(tokenMatches(src, BuiltInTypesString[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next;
                ((struct ParsingToken*)next->self)->TokenType = token_type;
                ((struct ParsingToken*)next->self)->Type = (enum BuiltInTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(BuiltInTypesString[i]);
                src+=strlen(BuiltInTypesString[i]);
                cur = next;
                printf("TYPE %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, index);
                index++;
                continue;
            }
        }
        // scan for operators
        for(int i = 0; i < IMPLEMENTED_OPERATORS; i++){
            if(tokenMatches(src, OperatorTypesString[i])){
                next = CreateLinkedListEntry(sizeof(struct ParsingToken));
                cur->next = next;
                ((struct ParsingToken*)next->self)->TokenType = token_operator;
                ((struct ParsingToken*)next->self)->Type = (enum BuiltInTypes)i;
                ((struct ParsingToken*)next->self)->Source = src;
                ((struct ParsingToken*)next->self)->Length = strlen(OperatorTypesString[i]);
                src+=strlen(OperatorTypesString[i]);
                cur = next;
                printf("OPERATOR %.*s (%i)\n", ((struct ParsingToken*)next->self)->Length, ((struct ParsingToken*)next->self)->Source, index);
                index++;
                continue;
            }
        }

        // scan for names
        if(isalpha(*src) || *src == '_'){
            next = CreateLinkedListEntry(sizeof(struct ParsingToken));
            cur->next = next;
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

            printf("NAME %.*s (%i)\n", len, s, index);
            index++;
            continue;
        }

        // scan for numbers
        if(isdigit(*src)){
            next = CreateLinkedListEntry(sizeof(struct ParsingToken));
            cur->next = next;
            int len = 0;
            ((struct ParsingToken*)next->self)->Source = src;
            while(isdigit(*src)){
                src++;
                len++;
            }

            ((struct ParsingToken*)next->self)->TokenType = token_number;
            ((struct ParsingToken*)next->self)->Length = len;

            printf("NUMBER %.*s (%i)\n", len, ((struct ParsingToken*)next->self)->Source, index);
            cur = next;
            index++;
            continue;
        }
    }

    // now that lexing is done. time to parse
    printf("PARSING\n");
    ParseFile(headEntry);

    return headEntry;
}