#include "linked.h"

struct LinkedListEntry* CreateLinkedListEntry(size_t memberSize){
    struct LinkedListEntry* ret = (struct LinkedListEntry*)malloc(sizeof(struct LinkedListEntry));
    memset(ret, 0, sizeof(struct LinkedListEntry*));
    ret->memberSize = memberSize;
    ret->self = malloc(memberSize);
    memset(ret->self, 0, memberSize);
    return ret;
}

struct LinkedListEntry* LinkedListOffset(struct LinkedListEntry* entry, int offset){
    for(int i = 0; i < offset && entry->next != NULL; i++, entry = entry->next ){}
    return entry->next != NULL ? entry : NULL;
}

void FreeLinkedList(struct LinkedListEntry* start){
    struct LinkedListEntry* cur = start;
    struct LinkedListEntry* next;
    while(next != NULL){
        next = cur->next;
        free(cur->self);
        free(cur);
        cur = next;
    }
}