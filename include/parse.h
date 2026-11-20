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

struct ParsingResult* ParseFile(struct LinkedListEntry* entry);
extern void DumpParsingLinkedEntry(struct LinkedListEntry* entry);