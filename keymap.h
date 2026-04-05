#ifndef KEYMAP_H
#define KEYMAP_H

#include "toml.h"
#include "helper.h"

struct TOMLKeyMap {
    char key[MAX_KEY_SIZE];
    int valueType;
    void *targetValue;
};

int TOMLApplyEntriesToKeyMap(struct TOMLEntry *entries, int entryCount, struct TOMLKeyMap *keyMap, int mapCount);

#endif
