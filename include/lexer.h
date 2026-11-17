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
#include "linked.h"
#include "parse.h"

struct ParsingLinkedEntry{
    struct ParsingLinkedEntry* last;
    struct ParsingLinkedEntry* next;
    struct ParsingToken* token;
};

struct ScopeIdentifier{
    char* name;
};

struct ScopeIdentifierLinkedEntry{
    struct ScopeIdentifier* last;
    struct ScopeIdentifier* next;
    struct ScopeIdentifier* this;
};

struct LinkedListEntry* LexizeFile(char* dir);