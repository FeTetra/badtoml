#include "toml.h"

/* Parsing */

static int TOMLParseSection(char *line, int size, char *buf) {
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

// TODO: Make this function return an error instead of i, we can skip the data this reads later anyways
// TODO: Make this use MemCpy or StrCpy function

static int TOMLParseKey(char *key, int size, struct TOMLEntry *entry) {
    int i = 0;

    while(i < size && IsAlpha(key[i])) {
        if (i < MAX_KEY_SIZE) {
            entry->key[i] = key[i]; // Copy key to buffer
        }

        i++;
    }

    entry->key[i] = '\0';
    return i;
}

static int TOMLGetValueType(char *value, int size) {
    int i = 0;
    int result = TOML_INVALID;

    if (value[0] == '\"') {
        while (++i < size) {
            if (value[i] == '\"') {
                result = TOML_STRING;
                i++;
                break;
            }
        }
    }

    if (value[0] == '\'') {
        while (++i < size) {
            if (value[i] == '\'') {
                result = TOML_STRING_LITERAL;
                i++;
                break;
            }
        }
    }

    int skip = (value[0] == '-' || value[0] == '+');
    if (IsNumeric(value[skip])) { 
        int decimalCount = 0;
        int intType = TOML_INT;
        i += skip;

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
                } else if (intType == TOML_INT_HEX && IsNumericHex(value[i])) {
                    i++;
                    continue;
                } else if (IsWhiteSpace(value[i]) || value[i] == '\0') {
                    break; 
                } 

                result = TOML_INVALID;
                break;
            }
            i++;
        }

        if (i <= size) {
            if (decimalCount == 1) {
                result = TOML_FLOAT;
            } else if (decimalCount == 0) {
                result = intType;
            }
        }
    }

    if (StrCmp(value, size, "true", 4)) {
        i += 5;
        result = TOML_BOOL;
    } else if (StrCmp(value, size, "false", 5)) {
        i += 6;
        result = TOML_BOOL;
    }

    while (i < size && result != TOML_INVALID) {
        if (!IsWhiteSpace(value[i]) && value[i] != '\0') {
            result = TOML_INVALID;
            return result;
        }
        i++;
    }

    return result;
}

static int TOMLParseValue(char *value, int size, struct TOMLEntry *entry) {
    int i = 0;

    switch (entry->valueType) {
        case TOML_INT:
            entry->value.intVal = StrToInt(value, size, 10);
            break;
        case TOML_INT_BIN:
            entry->value.intVal = StrToInt(value, size, 2);
            break;
        case TOML_INT_OCT:
            entry->value.intVal = StrToInt(value, size, 8);
            break;
        case TOML_INT_HEX:
            entry->value.intVal = StrToInt(value, size, 16);
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
            entry->value.strVal[i - 1] = '\0';
            break;
        case TOML_STRING_LITERAL: // Need to actually make a distinction between strings and string literals
            while (++i < MAX_VALUE_SIZE && i < size && value[i] != '\'') {
                entry->value.strVal[i - 1] = value[i];
            }
            entry->value.strVal[i - 1] = '\0';
            break;
    }

    return TOML_SUCCESS;
}

static int TOMLParseKeyValue(char *line, int size, struct TOMLEntry *entry) {
    int i = 0;

    i += TOMLParseKey(&line[i], (size - i), entry); 

    i += SkipWhitespace(&line[i], (size - i));

    if (line[i++] != '=') {
        return TOML_PARSE_FAIL; // No value
    }

    i += SkipWhitespace(&line[i], (size - i));
 
    entry->valueType = TOMLGetValueType(&line[i], (size - i));

    if (entry->valueType == TOML_INVALID) {
        return TOML_PARSE_FAIL;
    }
 
    if (TOMLParseValue(&line[i], (size - i), entry) != TOML_SUCCESS) {
        return TOML_PARSE_FAIL;
    }
    
    return TOML_SUCCESS;
}

static int TOMLParseLine(char *line, int size, struct TOMLEntry *entry) {
    int i = SkipWhitespace(line, size);

    if (line[i] == '#') {
        return TOML_PARSE_COMMENT;
    }
    if (line[i] == '[') {
        return TOML_PARSE_SECTION;
    }

    return TOMLParseKeyValue(line, size, entry);
}

int TOMLParseFileBuf(char *file, int size, struct TOMLEntry *entries, int count) {
    int i = 0;
    int j = 0;
    int remaining = 0;
    int result = TOML_SUCCESS;
    char currentSection[MAX_SECTION_SIZE] = "root";

    while (i < size && j < count) {
        i += SkipWhitespace(&file[i], (size - i));
        if (file[i] == '\0') {
            break; // In case of trailing newlines
        }

        remaining = NextLine(&file[i], (size - i)); 

        int error = TOMLParseLine(&file[i], remaining, &entries[j]);

        switch (error) {
            case TOML_SUCCESS:
                MemCpy(currentSection, MAX_SECTION_SIZE, entries[j].section, MAX_SECTION_SIZE);
                j++;
                break;
            case TOML_PARSE_COMMENT:
                break;
            case TOML_PARSE_SECTION:
                TOMLParseSection(&file[i], size, currentSection);
                break;
            case TOML_PARSE_FAIL:
                result = TOML_PARSE_FAIL;
                break;
        }

        i += remaining;
    }

    return result;
}


