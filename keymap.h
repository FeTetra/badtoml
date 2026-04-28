#ifndef KEYMAP_H
#define KEYMAP_H

#include "toml.h"

typedef struct {
    char key[MAX_KEY_SIZE];
    TOMLValueType valueType;
    void *targetValue;
} TOMLKeyMap;

TOMLErrno TOMLApplyEntriesToKeyMap(TOMLEntry *entries, size_t entryCount, TOMLKeyMap *keyMap, size_t mapCount);

#endif
