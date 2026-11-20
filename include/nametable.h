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
#include "types.h"
#include "linked.h"

struct NameTableEntry* DefEntryByName(struct LinkedListEntry* list, char* name, size_t len);
struct NameTableEntry* EntryByNameAndScope(struct LinkedListEntry* list, struct LinkedListEntry* scopeList, char* name, size_t len, int ScopeIndex);