#include "keymap.h"
 
TOMLErrno TOMLApplyEntriesToKeyMap(TOMLEntry *entries, size_t entryCount, TOMLKeyMap *keyMap, size_t mapCount) {
    for (int i = 0; i < mapCount; i++) {
        char *currentMapKey = keyMap[i].key;
        size_t currentMapKeyLen = strlen(currentMapKey);

        for (int j = 0; j < entryCount; j++) {
            char *currentEntryKey = entries[j].key;
            
            if (strncmp(currentEntryKey, currentMapKey, currentMapKeyLen) != 0) {
                continue;
            }

            if (entries[j].valueType != keyMap[i].valueType) {
                continue;
            }

            switch (keyMap[i].valueType) {
                case TOML_TYPE_INT:
                case TOML_TYPE_INT_BIN:
                case TOML_TYPE_INT_OCT:
                case TOML_TYPE_INT_HEX:
                    *(long long *)keyMap[i].targetValue = entries[j].value.intVal;
                    break;
                case TOML_TYPE_BOOL:
                    *(int *)keyMap[i].targetValue = entries[j].value.boolVal;
                    break;
                case TOML_TYPE_FLOAT:
                    *(double *)keyMap[i].targetValue = entries[j].value.floatVal;
                    break;
                case TOML_TYPE_STRING:
                case TOML_TYPE_LITERAL:
                    *(char **)keyMap[i].targetValue = entries[j].value.strVal;
                    break;
                case TOML_TYPE_INVALID:
                    break;
            }
        }
    }

    return TOML_ERRNO_SUCCESS;
}
