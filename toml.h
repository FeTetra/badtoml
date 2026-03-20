#include <stdio.h>

#include "helper.h"

#define MAX_SECTION_SIZE 64
#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 64 // For string values

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

/* Parsing */

// TODO: Return struct results for error handling

int TOMLParseKey(char *key, int size, struct TOMLEntry *entry) {
    int i = 0;

    while(i < size && IsAlpha(key[i])) {
        if (i < MAX_KEY_SIZE) {
            entry->key[i] = key[i]; // Copy key to buffer
        }

        i++;
    }

    return i;
}

int TOMLGetValueType(char *value, int size) {
    int i = 0;

    if (value[i] == '\"') {
        while (i++ < size) {
            if (value[i] == '\"') {
                if (!IsWhiteSpace(value[++i]) && value[i] != '\0') {
                    return TOML_INVALID;
                }
                return TOML_STRING;
            }
        }

        return TOML_INVALID;
    }
    if (value[i] == '\'') {
        while (i++ < size) {
            if (value[i] == '\'') {
                if (!IsWhiteSpace(value[++i]) && value[i] != '\0') {
                    return TOML_INVALID;
                }
                return TOML_STRING_LITERAL;
            }
        }

        return TOML_INVALID;
    }

    if (value[i] == '-' || value[i] == '+') {
        i++;
    }
    if (IsNumeric(value[i])) {
        int decimalCount = 0;
        int intType = TOML_INT;

        if (!IsNumeric(value[i + 1])) {
            switch (value[i + 1]) {
                case 'b':
                    intType = TOML_INT_BIN;
                    i += 2;
                    break;
                case 'o':
                    intType = TOML_INT_OCT;
                    i += 2;
                    break;
                case 'x':
                    intType = TOML_INT_HEX;
                    i += 2;
                    break;
            }
        }
        
        while (i < size) {
            if (!IsNumeric(value[i])) {
                if (value[i] == '.' && intType == TOML_INT) {
                    decimalCount++;
                    i++;
                    continue;
                } else if (IsWhiteSpace(value[i]) || value[i] == '\0') {
                    break; 
                }

                return TOML_INVALID;
            }
            i++;
        }

        if (i < size) {
            if (decimalCount == 1) {
                return TOML_FLOAT;
            } else if (decimalCount == 0) {
                return intType;
            }
        }

        return TOML_INVALID;
    }

    if (StrCmp(value, size, "true", 4) || StrCmp(value, size, "false", 5)) {
        return TOML_BOOL; // StrCmp checks if the string ends with whitespace
    }

    return TOML_INVALID;
}

int TOMLParseValue(char *value, int size, struct TOMLEntry *entry) {
    int i = 0;

    switch (entry->valueType) {
        case TOML_INT:
            entry->value.intVal = StrToInt(value, size, 10);
            break;
        case TOML_INT_BIN:
            entry->value.intVal = StrToInt(&value[i], size, 2);
            break;
        case TOML_INT_OCT:
            entry->value.intVal = StrToInt(&value[i], size, 8);
            break;
        case TOML_INT_HEX:
            entry->value.intVal = StrToInt(&value[i], size, 16);
            break;
        case TOML_BOOL:
            entry->value.boolVal = StrCmp(value, size, "true", 4); // Assume its false if it's bool type and not true
            break;
        case TOML_FLOAT:
            entry->value.floatVal = StrToFloat(value, size);
            break;
        case TOML_STRING:
            while (++i < MAX_VALUE_SIZE && i < size && value[i] != '\"') {
                entry->value.strVal[i - 1] = value[i];
            }
            entry->value.strVal[i + 1] = '\0';
            break;
        case TOML_STRING_LITERAL: // Need to actually make a distinction between strings and string literals
            while (++i < MAX_VALUE_SIZE && i < size && value[i] != '\'') {
                entry->value.strVal[i - 1] = value[i];
            }
            entry->value.strVal[i + 1] = '\0';
            break;
    }
    
    return TOML_SUCCESS;
}

int TOMLParseKeyValue(char *line, int size, struct TOMLEntry *entry) {
    int i = 0;

    i += SkipWhitespace(&line[i], (size - i)); 
 
    i += TOMLParseKey(&line[i], (size - i), entry);
 
    i += SkipWhitespace(&line[i], (size - i));

    if (line[i++] != '=') {
        return TOML_PARSE_FAIL; // No value
    }

    i += SkipWhitespace(&line[i], (size - i));

    // Parse value
    
    entry->valueType = TOMLGetValueType(&line[i], (size - i));

    if (entry->valueType == TOML_INVALID) {
        return TOML_PARSE_FAIL;
    }

    if (TOMLParseValue(&line[i], (size - i), entry) != TOML_SUCCESS) {
        return TOML_PARSE_FAIL;
    }
    
    return TOML_SUCCESS;
}

int TOMLParseLine(char *line, int size, struct TOMLEntry *entry) {
    int i = SkipWhitespace(line, size);

    if (line[i] == '#') {
        return TOML_PARSE_COMMENT;
    }
    if (line[i] == '[') {
        return TOML_PARSE_SECTION;
    }

    return TOMLParseKeyValue(line, size, entry);
}

int TOMLParseSection(char *line, int size, char *buf) {
    int i = SkipWhitespace(line, size);

    if (line[i] != '[') {
        return TOML_PARSE_FAIL;
    }

    int j = 0;
    while (i++ < size) {
        if (line[i] == ']') {
            if (i <= 1) {
                return TOML_PARSE_FAIL;
            }
            buf[j] = '\0';
            return TOML_SUCCESS;
        }

        buf[j++] = line[i];
    }

    return TOML_PARSE_FAIL;
}
