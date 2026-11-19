#include "parse.h"

bool tokenOrderMatches(struct LinkedListEntry* entry, enum ParsingTokenType* order, int orderLen){
    struct LinkedListEntry* cur = entry;
    for(int i = 0; cur != NULL && i < orderLen; i++, cur = cur->next){
        struct ParsingToken* curTok = (struct ParsingToken*)cur->self;
        if(curTok->TokenType != order[i]){ 
            //printf("%s != %s (%i)\n", TokenTypeStrings[(int)curTok->TokenType], TokenTypeStrings[order[i]], i); 
            return false; 
        }
    }
    return true;
}

bool tokenOrderMatchesReverse(struct LinkedListEntry* entry, enum ParsingTokenType* order, int orderLen){
    struct LinkedListEntry* cur = entry;
    for(int i = 0; i < orderLen && cur != NULL; i++, cur = cur->last){
        struct ParsingToken* curTok = (struct ParsingToken*)cur->self;
        if(curTok->TokenType != order[i]){
            //printf("%s != %s (%i)\n", TokenTypeStrings[(int)curTok->TokenType], TokenTypeStrings[order[i]], i);
            return false; 
        }
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
        enum ParsingTokenType order3[3] = {token_type, token_name, token_separator};
        enum ParsingTokenType order4[3] = {token_type, token_name, token_argumentEnd};
        if(tokenOrderMatches(entry, order3, 3) || tokenOrderMatches(entry, order4, 3)){
            return 3;
        }
        if((int)tok->Type < 4){
            // is a number
            enum ParsingTokenType setNumberOrder[5] = {token_type, token_name, token_set, token_number, token_end};
            return tokenOrderMatches(entry, setNumberOrder, 5) ? 5 : -1;
        }
        return (tokenOrderMatches(entry, order0, 3) || tokenOrderMatches(entry, order1, 3)) ? 3 : -1;
    }
    return 1;
}

struct ParsingResult* ParseFile(struct LinkedListEntry* entry){
    struct ParsingResult* result = (struct ParsingResult*)malloc(sizeof(struct ParsingResult));

    struct LinkedListEntry* scopes = CreateLinkedListEntry(sizeof(struct ParsingScope));
    struct LinkedListEntry* NameTable = CreateLinkedListEntry(sizeof(struct NameTableEntry));

    result->NameTable = NameTable;
    result->TokenList = entry;
    result->ScopeList = scopes;
        
    // SCOPING STEP
        // linked list with entry of struct ParsingScope
        // go through every token and set up the scopes, linking the scope index to each
    int length = 1;
    struct LinkedListEntry* curScope = scopes;
    struct LinkedListEntry* cur = entry;
    struct ParsingToken* lastSeenScopeOwner;
    while(cur != NULL){
        struct ParsingToken* curToken = (struct ParsingToken*)cur->self;
        // now assign the current token to the new scope
        curToken->Scope = ((struct ParsingScope*)curScope->self);
        cur = cur->next;
        if(
            curToken->TokenType == token_function ||
            curToken->TokenType == token_for ||
            curToken->TokenType == token_while
        ){
            // owner
            lastSeenScopeOwner = curToken;
        }else if(curToken->TokenType == token_scopeStart){
            struct LinkedListEntry* newScope = CreateLinkedListEntry(sizeof(struct ParsingScope));
            ((struct ParsingScope*)newScope->self)->index = length++;
            ((struct ParsingScope*)newScope->self)->parentIndex = ((struct ParsingScope*)curScope->self)->index;
            ((struct ParsingScope*)newScope->self)->owner = lastSeenScopeOwner;
            curScope->next = newScope;
            newScope->last = curScope;
            curScope = newScope;
        }else if(curToken->TokenType == token_scopeEnd){
            curScope = (struct LinkedListEntry*)(LinkedListOffset(scopes, ((struct ParsingScope*)curScope->self)->parentIndex));
        }
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
        // only corrects calls and arguments for now
    cur = entry;
    while(cur != NULL){
        struct ParsingToken* curTok = ((struct ParsingToken*)cur->self);
        if(curTok->Context != 0){
            cur = cur->next;
            continue;
        }
        if(curTok->TokenType == token_number){
            // possible orders for a number argument
                // NUMBER )
                // ( NUMBER
                // NUMBER ,
                // , NUMBER
            enum ParsingTokenType order0[2] = {token_number, token_argumentEnd};
            enum ParsingTokenType order1[2] = {token_argumentStart, token_number};
            enum ParsingTokenType order2[2] = {token_number, token_separator}; // FWD & BWD
            if( tokenOrderMatches(cur, order0, 2) || 
                tokenOrderMatches(cur, order1, 2) ||
                tokenOrderMatches(cur, order2, 2) ||
                tokenOrderMatchesReverse(cur, order0, 2)
            ){
                curTok->Context |= context_argument;
                printf("FOUND NUMBER ARGUMENT %.*s\n", curTok->Length, curTok->Source);
                cur = cur->next;
                continue;
            }
        }else if(curTok->TokenType == token_name){
            // possible orders for a name argument
                // NAME )
                // ( NAME
                // NAME ,
                // , NAME
            // specific impossible order that should be ignored if others match for a name argument
                // TYPE NAME
            enum ParsingTokenType order0[2] = {token_name, token_argumentEnd}; 
            enum ParsingTokenType order1[2] = {token_argumentStart, token_name};
            enum ParsingTokenType order2[2] = {token_name, token_separator}; // FWD & BWD
            enum ParsingTokenType impossible[2] = {token_name, token_type}; // BWD
            if( (tokenOrderMatches(cur, order0, 2) || 
                tokenOrderMatches(cur, order1, 2) ||
                tokenOrderMatches(cur, order2, 2) ||
                tokenOrderMatchesReverse(cur, order0, 2))
                &&
                !tokenOrderMatchesReverse(cur, impossible, 2)
            ){
                curTok->Context |= context_argument;
                cur = cur->next;
                printf("FOUND NAME ARGUMENT %.*s\n", curTok->Length, curTok->Source);
                continue;
            }
            // possible orders for a name call
                // NAME (
            // possible orders for a function definition
                // FUNCTION TYPE NAME (REVERSED)
            enum ParsingTokenType callOrder[2] = {token_name, token_argumentStart};
            enum ParsingTokenType fnDefOrder[3] = {token_name, token_type, token_function};
            bool fnDef = tokenOrderMatchesReverse(cur, fnDefOrder, 3);
            if(tokenOrderMatches(cur, callOrder, 2) && !fnDef){
                curTok->Context |= context_call;
                cur = cur->next;
                printf("FOUND NAMECALL %.*s\n", curTok->Length, curTok->Source);
                continue;
            }

            if(fnDef){
                curTok->Context |= (context_def | context_function);
                cur = cur->next;
                printf("FOUND FUNCTION DEF %.*s\n", curTok->Length, curTok->Source);
                continue;
            }
        }else if(curTok->TokenType == token_type){
            // possible orders for input argument
                // TYPE NAME SEPARATOR
                // TYPE NAME )
            enum ParsingTokenType order0[3] = {token_type, token_name, token_separator};
            enum ParsingTokenType order1[3] = {token_type, token_name, token_argumentEnd};
            if(tokenOrderMatches(cur, order0, 3) || tokenOrderMatches(cur, order1, 3)){
                curTok->Context |= context_argument | context_def;
                struct ParsingToken* nextTok = ((struct ParsingToken*)cur->next->self);
                nextTok->Type = curTok->Type;
                cur = cur->next;
                printf("FOUND INPUT ARG %.*s\n", curTok->Length, curTok->Source);
                continue;
            }
            // possible orders for return type
                // TYPE FN (REVERSE)
            enum ParsingTokenType returnOrder[2] = {token_type, token_function};
            if(tokenOrderMatchesReverse(cur, returnOrder, 2)){
                curTok->Context |= context_return_type;
                cur = cur->next;
                // set cur->next context to have function since its a name
                //((struct ParsingToken*)cur->self)->Context |= context_function;
                printf("FOUND RETURN TYPE %.*s\n", curTok->Length, curTok->Source);
                continue;
            }
            // possible orders for a variable definition
                // TYPE NAME
                    // will set the BuiltInTypes of NAME to TYPE
            enum ParsingTokenType varDef[2] = {token_type, token_name};
            if(tokenOrderMatches(cur, varDef, 2)){
                struct ParsingToken* nextTok = ((struct ParsingToken*)cur->next->self);
                nextTok->Type = curTok->Type;
                curTok->Context |= context_def;
                printf("FOUND VARIABLE DEF %.*s TYPE %.*s\n", nextTok->Length, nextTok->Source, curTok->Length, curTok->Source);
                continue;
            }
        }
        cur = cur->next;
    }
    // NAME TABLING
        // loop through and get function defs and their argc and arg types.
        // also get variable defs and their type
    struct LinkedListEntry* CurNameTable = NameTable;
    for(cur = entry; cur != NULL; cur = cur->next){
        struct ParsingToken* curTok = ((struct ParsingToken*)cur->self);
        if(curTok->TokenType == token_function){
            // wooo yay new function def fuckkkk
            struct ParsingToken* nameTok = ((struct ParsingToken*)cur->next->next->self);
            if(nameTok->TokenType != token_name || !(nameTok->Context & context_argument | context_def)){
                printf("Error: expected token_name but got %i\n", nameTok->TokenType);
                break;
            }
            struct LinkedListEntry* NewNameTable = CreateLinkedListEntry(sizeof(struct NameTableEntry));
            struct NameTableEntry* nte = ((struct NameTableEntry*)NewNameTable->self);
            nte->name = nameTok->Source;
            nte->nameLength = nameTok->Length;
            nte->Type = tabledef_function;
            // next token should be token_type with context flag of context_return_type
            struct ParsingToken* retTypeTok = ((struct ParsingToken*)cur->next->self);
            if(retTypeTok->TokenType != token_type || !(retTypeTok->Context & context_return_type)){
                printf("Error: expect token_type but got %i\n", retTypeTok->TokenType);
                break;
            }

            // okay now loop through and get each argument and its type until we hit a token_argumentEnd
            int argc = 0;
            struct LinkedListEntry* curArgs;
            struct ParsingToken* curArgT;
            // cur is fn
            // fn none main(i8
            // quad next would hit the i8
            for(curArgs = cur->next->next->next->next; curArgs != NULL; curArgs = curArgs->next){
                curArgT = ((struct ParsingToken*)curArgs->self);
                if(curArgT->TokenType == token_argumentEnd){
                    nte->argc = argc;
                    break;
                }else if(curArgT->TokenType == token_type && (curArgT->Context & (context_argument | context_def))){
                    // is an argument
                    printf("FOUND ARG\n");
                    nte->argv[argc] = ((struct ParsingToken*)curArgs->next->self); // argv[argc] points to the name
                    argc++;
                }
            }

            // finally go from final arg to token_argumentEnd to determine the status
                // okay so at final arg name, these are the combination matches
                // DEFINITION
                    // NAME (w/ context_argument) ARGUMENTEND SCOPESTART
                // PROMISED
                    // NAME (w/ context_argument) ARGUMENTEND END
            struct LinkedListEntry* argEnd = NULL;
            struct LinkedListEntry* curArgEnd;
            for(curArgEnd = cur; curArgEnd != NULL; curArgEnd = curArgEnd->next){
                struct ParsingToken* tok = (struct ParsingToken*)curArgEnd->self;
                if(tok->TokenType == token_argumentEnd){
                    printf("ooo end\n");
                    argEnd = curArgEnd;
                    break;
                }
            }

            // wooo found it
            // so now check it
            enum ParsingTokenType defOrder[2] = {token_argumentEnd, token_scopeStart};
            enum ParsingTokenType promiseOrder[2] = {token_argumentEnd, token_end};
            if(tokenOrderMatches(argEnd, defOrder, 2)){
                // is a def
                nte->Status = table_definition;
                printf("DEFINITION\n");
            }else if(tokenOrderMatches(argEnd, promiseOrder, 2)){
                // is a promise
                nte->Status = table_promise;
                printf("PROMISE\n");
            }else{
                printf("ERROR: invalid function definition combination\n");
                return result;
            }

            CurNameTable->next = NewNameTable;
            NewNameTable->last = CurNameTable;
            CurNameTable = NewNameTable;
        }else if(curTok->TokenType == token_type){
            // must be a variable def, no explicit conversions (yet). much simpler since no args
            if(curTok->Context & context_return_type){
                continue;
            }
            struct ParsingToken* nameTok = ((struct ParsingToken*)cur->next->self);
            if(nameTok->TokenType != token_name || !(nameTok->Context & context_argument | context_def)){
                printf("Error: expected token_name but got %i\n", nameTok->TokenType);
                break;
            }
            struct LinkedListEntry* NewNameTable = CreateLinkedListEntry(sizeof(struct NameTableEntry));
            struct NameTableEntry* nte = ((struct NameTableEntry*)NewNameTable->self);
            nte->name = nameTok->Source;
            nte->nameLength = nameTok->Length;
            nte->Type = tabledef_variable;
            nte->argc = 1;
            nte->argv[0] = nameTok;

            CurNameTable->next = NewNameTable;
            NewNameTable->last = CurNameTable;
            CurNameTable = NewNameTable;
        }else if(curTok->TokenType == token_name){
            if(curTok->Context & context_call){
                // okay sweet its a call so now we add it to the table as a reference
                
            }
        }
    }
    // loop through nametable and get all data
    struct LinkedListEntry* curTable = NameTable;
    for(curTable = NameTable; curTable != NULL; curTable = curTable->next){
        struct NameTableEntry* curNTE = (struct NameTableEntry*)curTable->self;
        if(curNTE->Type == tabledef_function){
            printf("FUNCTION %.*s ARGC %i\n", curNTE->nameLength, curNTE->name, curNTE->argc);
            for(int i = 0; i < curNTE->argc; i++){
                printf("\t%s %.*s\n", BuiltInTypesString[(int)curNTE->argv[i]->Type], curNTE->argv[i]->Length, curNTE->argv[i]->Source);
                struct ParsingToken* tok = curNTE->argv[i];
                //printf("\t%.*s (%s)\n", tok->Length, tok->Source);
            }
        }else if(curNTE->Type == tabledef_variable){
            printf("VARIABLE %.*s TYPE %s\n", curNTE->nameLength, curNTE->name, BuiltInTypesString[(int)curNTE->argv[0]->Type]);
        }
    }

    return result;
}