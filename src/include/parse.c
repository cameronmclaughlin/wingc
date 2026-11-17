#include "parse.h"

bool tokenOrderMatches(struct LinkedListEntry* entry, enum ParsingTokenType* order, int orderLen){
    struct LinkedListEntry* cur = entry;
    for(int i = 0; cur != NULL && i < orderLen; i++, cur = cur->next){
        struct ParsingToken* curTok = (struct ParsingToken*)cur->self;
        if(curTok->TokenType != order[i]){ printf("%s != %s (%i)\n", TokenTypeStrings[(int)curTok->TokenType], TokenTypeStrings[order[i]], i); return false; }
    }
    return true;
}

int checkExpecting(struct LinkedListEntry* entry){
    //DumpParsingLinkedEntry(entry);
    struct ParsingToken* tok = (struct ParsingToken*)entry->self; 
    if(tok->TokenType == token_invalid){return -1;}
    else if(tok->TokenType == token_function){
        printf("TOKEN FUNCTION\n");
        enum ParsingTokenType order[4] = {token_function, token_type, token_name, token_argumentStart};
        return tokenOrderMatches(entry, order, 4) ? 4 : -1;
    }else if(tok->TokenType == token_name){
        printf("TOKEN NAME\n");
        enum ParsingTokenType order0[2] = {token_name, token_operator};
        enum ParsingTokenType order1[2] = {token_name, token_argumentStart};
        return (tokenOrderMatches(entry, order0, 2) || tokenOrderMatches(entry, order1, 2)) ? 2 : -1;
    }else if(tok->TokenType == token_type){
        printf("TOKEN TYPE\n");
        enum ParsingTokenType order0[3] = {token_type, token_name, token_end};
        enum ParsingTokenType order1[3] = {token_type, token_name, token_set};
        if((int)tok->Type < 4){
            // is a number
            enum ParsingTokenType setNumberOrder[5] = {token_type, token_name, token_set, token_number, token_end};
            return tokenOrderMatches(entry, setNumberOrder, 5) ? 5 : -1;
        }
        return (tokenOrderMatches(entry, order0, 3) || tokenOrderMatches(entry, order1, 3)) ? 3 : -1;
    }
    return 1;
}

struct LinkedListEntry* ParseFile(struct LinkedListEntry* entry){
    // SCOPING STEP
        // linked list with entry of struct ParsingScope
        // go through every token and set up the scopes, linking the scope index to each
    struct LinkedListEntry* scopes = CreateLinkedListEntry(sizeof(struct ParsingScope));
    int length = 1;
    struct LinkedListEntry* curScope = scopes;
    struct LinkedListEntry* cur = entry;
    while(cur != NULL){
        struct ParsingToken* curToken = (struct ParsingToken*)cur->self;
        if(curToken->TokenType == token_scopeStart){
            struct LinkedListEntry* newScope = CreateLinkedListEntry(sizeof(struct ParsingScope));
            ((struct ParsingScope*)newScope->self)->index = length++;
            ((struct ParsingScope*)newScope->self)->parentIndex = ((struct ParsingScope*)curScope->self)->index;
            curScope->next = newScope;
            newScope->last = curScope;
            curScope = newScope;
        }else if(curToken->TokenType == token_scopeEnd){
            curScope = (struct LinkedListEntry*)(LinkedListOffset(scopes, ((struct ParsingScope*)curScope->self)->parentIndex));
        }
        // now assign the current token to the new scope
        curToken->ScopeIndex = ((struct ParsingScope*)curScope->self)->index;
        cur = cur->next;
    }
        // print scopes
    cur = scopes;
    while(cur != NULL){
        struct ParsingScope* curScope = (struct ParsingScope*)cur->self;
        printf("SCOPE %i PARENTED TO %i\n", curScope->index, curScope->parentIndex);
        cur = cur->next;
    }

    // ORDERING STEP
    int index = 0;
    cur = entry;
    while(cur != NULL){
        int expect = checkExpecting(cur);
        if(expect == -1){ printf("ERR TOKEN %s at INDEX %i\n", TokenTypeStrings[((struct ParsingToken*)cur->self)->TokenType], index); return NULL; }
        index+=expect;
        cur = LinkedListOffset(entry, index);
    }
    // CONTEXTUALIZATION STEP
        // sets context flags for tokens
        // also corrects the scope for arguments
        

    // NAME TABLING
        // loop through and get function defs and their argc and arg types.
        // also get variable defs and their type

    return entry;
}