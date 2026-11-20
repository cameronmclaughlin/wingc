#include "nametable.h"

struct NameTableEntry* DefEntryByName(struct LinkedListEntry* list, char* name, size_t len){
    struct LinkedListEntry* cur;
    for(cur = list; cur != NULL; cur = cur->next){
        struct NameTableEntry* nte = (struct NameTableEntry*)cur->self;
        //if(nte->name != NULL){ printf("%.*s COMP %.*s (%i COMP %i)\n", len, name, nte->nameLength, nte->name, len, nte->nameLength); }
        if(nte->nameLength != len){ continue; }
        int targLen = (len <= nte->nameLength) ? len : nte->nameLength;
        if(nte->name != NULL && (strncmp(name, nte->name, targLen) == 0) && nte->Status != table_reference){
            return nte;
        }
    }
    return NULL;
}

struct NameTableEntry* EntryByNameAndScope(struct LinkedListEntry* list, struct LinkedListEntry* scopeList, char* name, size_t len, int ScopeIndex){
    struct LinkedListEntry* cur;
    printf("SEARCH %.*s (%i)\n", len, name, ScopeIndex);
    for(cur = list; cur != NULL; cur = cur->next){
        struct NameTableEntry* nte = (struct NameTableEntry*)cur->self;
        //if(nte->name != NULL){ printf("%.*s COMP %.*s (%i COMP %i)\n", len, name, nte->nameLength, nte->name, len, nte->nameLength); }
        if(nte->nameLength != len){ continue; }
        int targLen = (len <= nte->nameLength) ? len : nte->nameLength;
        printf("SEE %.*s (%i)\n", nte->owner->Length, nte->owner->Source, nte->owner->Scope->index);
        if(nte->name != NULL && (strncmp(name, nte->name, targLen) == 0)){
            if(nte->owner->Scope->index == ScopeIndex){
                return nte;
            }else{
                // backtrack until it finds it in the scope
                struct LinkedListEntry* curScope = LinkedListOffset(scopeList, nte->owner->Scope->index);
                while(true){
                    struct LinkedListEntry* parentScopeList = LinkedListOffset(scopeList, ((struct ParsingScope*)curScope->self)->parentIndex);
                    struct ParsingScope* parentScope = ((struct ParsingScope*)parentScopeList->self);
                    
                    if(parentScope->index == ScopeIndex){
                        return nte;
                    }else if(parentScope->index == 0){
                        break;
                    }
                    curScope = parentScopeList;
                }
            }
        }
    }
    return NULL;
}