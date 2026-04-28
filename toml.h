#ifndef TOML_H
#define TOML_H

#include <stddef.h>

#include "tokenizer.h"
#include "helper.h"

#define MAX_SECTION_SIZE 64
#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 64 // For string values
#define FLOAT_ROUND_WRITE 5 // Round to 5 places

typedef enum {
    TOML_ERRNO_SUCCESS,
    TOML_ERRNO_PARSE_FAIL,
    TOML_ERRNO_EOF,
    TOML_ERRNO_SECTION,
    TOML_ERRNO_INVALID,
} TOMLErrno;

typedef enum {
    TOML_TYPE_INT,
    TOML_TYPE_INT_BIN,
    TOML_TYPE_INT_OCT,
    TOML_TYPE_INT_HEX,
    TOML_TYPE_BOOL,
    TOML_TYPE_FLOAT,
    TOML_TYPE_STRING,
    TOML_TYPE_LITERAL,
    TOML_TYPE_INVALID,
    //...
} TOMLValueType;

typedef struct {
    char section[MAX_SECTION_SIZE];
    char key[MAX_KEY_SIZE];

    TOMLValueType valueType;
    union {
        char strVal[MAX_VALUE_SIZE];
        int boolVal;
        long long intVal;
        double floatVal;
    } value;
} TOMLEntry;

TOMLErrno TOMLReadLine(Lexer *l, TOMLEntry *entry);

void TOMLReadBuffer(Lexer *l, TOMLEntry *entries, size_t count);

int TOMLMakeKeyValueFromEntry(TOMLEntry entry, char *buf, size_t size);

int TOMLMakeBufFromEntries(TOMLEntry *entries, size_t count, char *buf, size_t size);

#endif
