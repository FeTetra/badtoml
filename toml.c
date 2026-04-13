#include "toml.h"

/* Parsing */

static int TOMLParseSection(char *line, size_t size, char *buf) {
    int i = Skip(line, size);

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
// TODO: Make this use memcpy or StrCpy function

static int TOMLParseKey(char *key, size_t size, TOMLEntry *entry) {
    size_t i = 0;

    while(i < size && isalpha(key[i])) {
        if (i < MAX_KEY_SIZE) {
            entry->key[i] = key[i]; // Copy key to buffer
        }

        i++;
    }

    entry->key[i] = '\0';
    return i;
}

static int TOMLGetValueType(char *value, size_t size) {
    size_t i = 0;
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
    if (isdigit(value[skip])) { 
        int decimalCount = 0;
        int intType = TOML_INT;
        i += skip;

        if (!isdigit(value[i + 1])) {
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
            if (!isdigit(value[i])) {
                if (value[i] == '.' && intType == TOML_INT) {
                    decimalCount++;
                    i++;
                    continue;
                } else if (intType == TOML_INT_HEX && isxdigit(value[i])) {
                    i++;
                    continue;
                } else if (isspace(value[i]) || value[i] == '\0') {
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

    if (strncmp(value, "true", 4) >= 0) {
        i += 5;
        result = TOML_BOOL;
    } else if (strncmp(value, "false", 5) >= 0) {
        i += 6;
        result = TOML_BOOL;
    }

    while (i < size && result != TOML_INVALID) {
        if (!isspace(value[i]) && value[i] != '\0') {
            result = TOML_INVALID;
            return result;
        }
        i++;
    }

    return result;
}

static int TOMLParseValue(char *value, size_t size, TOMLEntry *entry) {
    size_t i = 0;

    switch (entry->valueType) {
        case TOML_INT:
            entry->value.intVal = StrToLL(value, size, 10);
            break;
        case TOML_INT_BIN:
            entry->value.intVal = StrToLL(value, size, 2);
            break;
        case TOML_INT_OCT:
            entry->value.intVal = StrToLL(value, size, 8);
            break;
        case TOML_INT_HEX:
            entry->value.intVal = StrToLL(value, size, 16);
            break;
        case TOML_BOOL:
            entry->value.boolVal = strncmp(value, "true", 4) <= 0; // Assume its false if it's bool type and not true
            break;
        case TOML_FLOAT:
            entry->value.floatVal = StrToDouble(value, size);
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

static int TOMLParseKeyValue(char *line, size_t size, TOMLEntry *entry) {
    size_t i = 0;

    i += TOMLParseKey(&line[i], (size - i), entry); 

    i += Skip(&line[i], (size - i));

    if (line[i++] != '=') {
        return TOML_PARSE_FAIL; // No value
    }

    i += Skip(&line[i], (size - i));
 
    entry->valueType = TOMLGetValueType(&line[i], (size - i));

    if (entry->valueType == TOML_INVALID) {
        return TOML_PARSE_FAIL;
    }
 
    if (TOMLParseValue(&line[i], (size - i), entry) != TOML_SUCCESS) {
        return TOML_PARSE_FAIL;
    }
    
    return TOML_SUCCESS;
}

static int TOMLParseLine(char *line, size_t size, TOMLEntry *entry) {
    int i = Skip(line, size);

    if (line[i] == '#') {
        return TOML_PARSE_COMMENT;
    }
    if (line[i] == '[') {
        return TOML_PARSE_SECTION;
    }

    return TOMLParseKeyValue(line, size, entry);
}

int TOMLParseFileBuf(char *file, size_t size, TOMLEntry *entries, int count) {
    size_t i = 0;
    int j = 0;
    int remaining = 0;
    int result = TOML_SUCCESS;
    char currentSection[MAX_SECTION_SIZE] = "root";

    while (i < size && j < count) {
        i += Skip(&file[i], (size - i));
        if (file[i] == '\0') {
            break; // In case of trailing newlines
        }

        remaining = NextLine(&file[i], (size - i)); 

        int error = TOMLParseLine(&file[i], remaining, &entries[j]);

        switch (error) {
            case TOML_SUCCESS:
                memcpy(entries[j].section, currentSection, MAX_SECTION_SIZE);
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

/* Serialization */

int TOMLCreateValueFromEntry(TOMLEntry entry, char *buf, size_t size) {
    switch (entry.valueType) {
        case TOML_INT:
            SLLToStr(buf, size, entry.value.intVal, 10);
            break;
        case TOML_INT_BIN:
            SLLToStr(buf, size, entry.value.intVal, 2);
            break;
        case TOML_INT_OCT:
            SLLToStr(buf, size, entry.value.intVal, 8);
            break;
        case TOML_INT_HEX:
            SLLToStr(buf, size, entry.value.intVal, 16);
            break;

        case TOML_BOOL:
            if (entry.value.boolVal) {
                memcpy(buf, "true", 4);
            }
            else {
                memcpy(buf, "false", 5);
            }
            break;

        case TOML_STRING:
        case TOML_STRING_LITERAL:; // yikes
            int valueSize = strlen(entry.value.strVal);
            memcpy(buf, entry.value.strVal, valueSize);
            break;
        
        case TOML_FLOAT:
            DoubleToStr(buf, size, entry.value.floatVal, FLOAT_ROUND_WRITE);
            break;
    }

    return 1;
}

int TOMLCreateKeyValueFromEntry(TOMLEntry entry, char *buf, size_t size) {
    size_t i = 0;

    memset(buf, '\0', size); // Laziness

    int keySize = strlen(entry.key);
    memcpy(&buf[i], entry.key, keySize);
    i += keySize;
    memcpy(&buf[i], " = ", 3);
    i += 3;

    TOMLCreateValueFromEntry(entry, &buf[i], (size - i));

    return 1;
}

int TOMLCreateFileFromEntries(TOMLEntry *entries, int count, char *buf, size_t size) {
    size_t i = 0;
    int j = 0;

    char *currentSection = NULL;

    memset(buf, '\0', size); // Double laziness

    // This is all so ugly
    while (j < count && i < size) {
        int sectionSize = strlen(entries[j].section);
        if (currentSection == NULL || strncmp(entries[j].section, currentSection, sectionSize) > 0) {
            buf[i++] = '[';
            memcpy(&buf[i], entries[j].section, sectionSize);
            currentSection = &buf[i];
            i += sectionSize;
            memcpy(&buf[i], "]\n", 2);
            i += 2;
        }

        TOMLCreateKeyValueFromEntry(entries[j], &buf[i], (size - i));
        while (buf[i] != '\0' && i < size) { i++; }
        buf[i++] = '\n';
        j++;
    }

    return 1;
}

