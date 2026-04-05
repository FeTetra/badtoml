#include "keymap.h"
#include <stdio.h>
 
int TOMLApplyEntriesToKeyMap(struct TOMLEntry *entries, int entryCount, struct TOMLKeyMap *keyMap, int mapCount) {
    for (int i = 0; i < mapCount; i++) {
        char *currentMapKey = keyMap[i].key;
        int currentMapKeyLen = StrLen(currentMapKey); 
        printf("currentMapKey: %s\n", currentMapKey);

        for (int j = 0; j < entryCount; j++) {
            char *currentEntryKey = entries[j].key;
            int currentEntryKeyLen = StrLen(currentEntryKey);
            
            if (!StrCmp(currentEntryKey, currentEntryKeyLen, currentMapKey, currentMapKeyLen)) {
                continue;
            }

            if (entries[j].valueType != keyMap[i].valueType) {
                continue;
            }

            switch (keyMap[i].valueType) {
                case TOML_INT:
                case TOML_INT_BIN:
                case TOML_INT_OCT:
                case TOML_INT_HEX:
                    *(long long *)keyMap[i].targetValue = entries[i].value.intVal;
                    break;
                case TOML_BOOL:
                    *(int *)keyMap[i].targetValue = entries[i].value.boolVal;
                    break;
                case TOML_FLOAT:
                    *(double *)keyMap[i].targetValue = entries[i].value.floatVal;
                    break;
                case TOML_STRING:
                case TOML_STRING_LITERAL:
                    keyMap[i].targetValue = (void *)entries[i].value.strVal;
                    break;
                case TOML_INVALID:
                    break;
            }
        }
    }

    return TOML_SUCCESS;
}
