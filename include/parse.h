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
#include "fpemdas.h"
#include "types.h"
#include "nametable.h"

#define IMPLEMENTED_TOKENS 11
#define IMPLEMENTED_CONDITIONALS 6
#define IMPLEMENTED_TYPES 9
#define IMPLEMENTED_OPERATORS 6

struct ParsingResult* ParseFile(struct LinkedListEntry* entry);
extern void DumpParsingLinkedEntry(struct LinkedListEntry* entry);