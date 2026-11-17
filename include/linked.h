#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

struct LinkedListEntry{
    struct LinkedListEntry* last;
    struct LinkedListEntry* next;
    void* self;
    size_t memberSize;
};

struct LinkedListEntry* CreateLinkedListEntry(size_t memberSize);
struct LinkedListEntry* LinkedListOffset(struct LinkedListEntry* entry, int offset);
void FreeLinkedList(struct LinkedListEntry* start);