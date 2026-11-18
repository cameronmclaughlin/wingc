#include "nametable.h"

struct NameTableEntry* EntryByName(struct LinkedListEntry* list, char* name, size_t len){
    struct LinkedListEntry* cur;
    for(cur = list; cur != NULL; cur = cur->next){
        struct NameTableEntry* nte = (struct NameTableEntry*)cur->self;
        //if(nte->name != NULL){ printf("%.*s COMP %.*s (%i COMP %i)\n", len, name, nte->nameLength, nte->name, len, nte->nameLength); }
        int targLen = (len <= nte->nameLength) ? len : nte->nameLength;
        if(nte->name != NULL && (strncmp(name, nte->name, targLen) == 0)){
            return nte;
        }
    }
    return NULL;
}