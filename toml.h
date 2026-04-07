#ifndef TOML_H
#define TOML_H

#include "helper.h"

#define MAX_SECTION_SIZE 64
#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 64 // For string values
#define FLOAT_ROUND_WRITE 5 // Round to 5 places

struct TOMLEntry {
    char section[MAX_SECTION_SIZE];
    char key[MAX_KEY_SIZE];

    int valueType;
    union {
        char strVal[MAX_VALUE_SIZE];
        int boolVal;
        long long intVal;
        double floatVal;
    } value;
};

enum TOMLErrno {
    TOML_SUCCESS,
    TOML_PARSE_COMMENT,
    TOML_PARSE_SECTION,
    TOML_PARSE_FAIL,
    //...
};

enum TOMLValueType {
    TOML_INT,
    TOML_INT_BIN,
    TOML_INT_OCT,
    TOML_INT_HEX,
    TOML_BOOL,
    TOML_FLOAT,
    TOML_STRING,
    TOML_STRING_LITERAL,
    TOML_INVALID,
    //...
};

int TOMLParseFileBuf(char *file, int size, struct TOMLEntry *entries, int count);

int TOMLCreateKeyValueFromEntry(struct TOMLEntry entry, char *buf, int size);

int TOMLCreateFileFromEntries(struct TOMLEntry *entries, int count, char *buf, int size);

#endif
