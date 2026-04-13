#ifndef KEYMAP_H
#define KEYMAP_H

#include <string.h>
#include <ctype.h>

#include "toml.h"
#include "helper.h"

typedef struct TOMLKeyMap {
    char key[MAX_KEY_SIZE];
    int valueType;
    void *targetValue;
} TOMLKeyMap;

int TOMLApplyEntriesToKeyMap(TOMLEntry *entries, int entryCount, TOMLKeyMap *keyMap, int mapCount);

#endif
