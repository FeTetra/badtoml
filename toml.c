#include "toml.h"

/* Deserialization */

// Return 1 if we reach a newline token without 
// hitting any other tokens on the way, return 0 otherwise
int TOMLNextLine(Lexer *l) {
    int result = 1;
    for (;;) {
        Token current = NextToken(l);
        if (current.type != TOKEN_NEWLINE && current.type != TOKEN_EOF) {
            result = 0;
            continue;
        }

        break;
    }

    return result;
}

void TOMLCopyValue(Token *t, TOMLEntry *entry) {
    switch (t->type) {
        case TOKEN_STRING_LITERAL:
        case TOKEN_STRING:
            entry->valueType = TOML_TYPE_STRING;
            memcpy(entry->value.strVal, (t->start + 1), (t->length - 2)); // Trim quotes
            entry->value.strVal[t->length - 2] = '\0';
            break;
        case TOKEN_INT:
            if (t->intType == TOKEN_INT_DEC) entry->valueType = TOML_TYPE_INT;
            if (t->intType == TOKEN_INT_BIN) entry->valueType = TOML_TYPE_INT_BIN;
            if (t->intType == TOKEN_INT_OCT) entry->valueType = TOML_TYPE_INT_OCT;
            if (t->intType == TOKEN_INT_HEX) entry->valueType = TOML_TYPE_INT_HEX;
            entry->value.intVal = StrToInt(t->start, t->length + 1, t->intType);
            break;
        case TOKEN_FLOAT:
            entry->valueType = TOML_TYPE_FLOAT;
            entry->value.floatVal = StrToFloat(t->start, t->length + 1);
            break;
        case TOKEN_TRUE:
            entry->valueType = TOML_TYPE_BOOL;
            entry->value.boolVal = 1;
            break;
        case TOKEN_FALSE:
            entry->valueType = TOML_TYPE_BOOL;
            entry->value.boolVal = 0;
            break;
        case TOKEN_LBRACKET:
            return; // TODO: Implement lists
        case TOKEN_LBRACE:
            return; // TODO: Implement tables (dont wanna)
        case TOKEN_DATE:
            return; // TODO: Implement dates
        default:
            return; // FAIL
    }
}

TOMLErrno TOMLReadLine(Lexer *l, TOMLEntry *entry) {
    Token current = NextToken(l);
    TOMLErrno result = TOML_ERRNO_INVALID;

    if (result == TOML_ERRNO_INVALID && current.type == TOKEN_EOF) 
        result = TOML_ERRNO_EOF; // END

    if (result == TOML_ERRNO_INVALID && current.type == TOKEN_LBRACKET) {
        current = NextToken(l);
        if (current.type != TOKEN_IDENTIFIER)
            result = TOML_ERRNO_PARSE_FAIL; // FAIL, not a section
        if (NextToken(l).type != TOKEN_RBRACKET)
            result = TOML_ERRNO_PARSE_FAIL; // FAIL, not a section

        memcpy(entry->section, current.start, current.length); // Copy section
        entry->section[current.length] = '\0';

        result = TOML_ERRNO_SECTION; // SUCCESS, read section
    }

    if (result == TOML_ERRNO_INVALID && current.type == TOKEN_IDENTIFIER) {
        memcpy(entry->key, current.start, current.length); // Copy key
        entry->key[current.length] = '\0';

        if (NextToken(l).type != TOKEN_EQUAL) 
            result = TOML_ERRNO_PARSE_FAIL; // FAIL, not a key/value

        current = NextToken(l);

        TOMLCopyValue(&current, entry); // Copy value

        result = TOML_ERRNO_SUCCESS; // SUCCESS, read key/value
    }

    if (!TOMLNextLine(l))
        result = TOML_ERRNO_PARSE_FAIL;

    return result; // FAIL, invalid type
}

void TOMLReadBuffer(Lexer *l, TOMLEntry *entries, size_t count) {
    char *currentSection = NULL;
    unsigned int currentSectionLen = 0;
    unsigned int i = 0;
    while (i < count) {
        int err = TOMLReadLine(l, &entries[i]);
        if (err == TOML_ERRNO_PARSE_FAIL) {
            entries[i].valueType = TOML_TYPE_INVALID; // TODO: Better error handling inside toml entries
        }
        if (err == TOML_ERRNO_SECTION && (currentSection == NULL || strncmp(currentSection, entries[i].section, currentSectionLen) != 0)) {
            currentSection = entries[i].section;
            currentSectionLen = strlen(entries[i].section);
        } else {
            memcpy(entries[i].section, currentSection, MAX_SECTION_SIZE);
            i++;
        }
        if (err == TOML_ERRNO_EOF) break;
    }

    return;
}

/* Serialization */

int TOMLCreateValueFromEntry(TOMLEntry entry, char *buf, size_t size) {
    switch (entry.valueType) {
        case TOML_TYPE_INT:
            SIntToStr(buf, size, entry.value.intVal, 10);
            break;
        case TOML_TYPE_INT_BIN:
            SIntToStr(buf, size, entry.value.intVal, 2);
            break;
        case TOML_TYPE_INT_OCT:
            SIntToStr(buf, size, entry.value.intVal, 8);
            break;
        case TOML_TYPE_INT_HEX:
            SIntToStr(buf, size, entry.value.intVal, 16);
            break;

        case TOML_TYPE_BOOL:
            if (entry.value.boolVal) {
                memcpy(buf, "true", 4);
            }
            else {
                memcpy(buf, "false", 5);
            }
            break;

        case TOML_TYPE_STRING:
        case TOML_TYPE_LITERAL:; // yikes
            *buf++ = '"'; // evil as hell
            size_t valueSize = strlen(entry.value.strVal);
            memcpy(buf, entry.value.strVal, valueSize);
            *(buf + valueSize) = '"'; // gross
            break;
        
        case TOML_TYPE_FLOAT:
            FloatToStr(buf, size, entry.value.floatVal, FLOAT_ROUND_WRITE);
            break;

        default:
            return 0; // TODO: Handle this PLEASE
    }

    return 1;
}

int TOMLMakeKeyValueFromEntry(TOMLEntry entry, char *buf, size_t size) {
    size_t i = 0;

    memset(buf, '\0', size); // Laziness

    // TODO: Handle overflow cases
    size_t keySize = strlen(entry.key);
    if (keySize + 3 < size) {
        memcpy(&buf[i], entry.key, keySize);
        i += keySize;
        memcpy(&buf[i], " = ", 3);
        i += 3;
    }

    TOMLCreateValueFromEntry(entry, &buf[i], (size - i));

    return 1;
}

int TOMLMakeBufFromEntries(TOMLEntry *entries, size_t count, char *buf, size_t size) {
    size_t i = 0;
    size_t j = 0;

    char *currentSection = NULL;

    memset(buf, '\0', size); // Double laziness

    // This is all so ugly
    while (j < count && i < size) {
        size_t sectionSize = strlen(entries[j].section);
        if (currentSection == NULL || strncmp(entries[j].section, currentSection, sectionSize) > 0) {
            buf[i++] = '[';
            if (sectionSize + 2 < size) {
                memcpy(&buf[i], entries[j].section, sectionSize);
                currentSection = &buf[i];
                i += sectionSize;
                memcpy(&buf[i], "]\n", 2);
                i += 2;
            }
        }

        TOMLMakeKeyValueFromEntry(entries[j], &buf[i], (size - i));
        while (buf[i] != '\0' && i < size) { i++; }
        buf[i++] = '\n';
        j++;
    }

    return 1;
}
