/* Type definitions */

struct TOMLEntry {
    char Section[32];
    char Key[32];
    union {
        long long IntVal;
        int BoolVal;
        char StrVal[64];
    } Value;
};

/* Type (de)serialization */

// This works exclusively with integral values, as it takes advantage of integer arithmetic
// Note: at the moment this only outputs big endian
int IntToStr(char *buf, int bufSize, unsigned long long n, int sign, int base) {
    if (base < 2 || base > 16 || base % 2) {
        return 0; // Fail, unsupported base
    }

    // Theres probably a more elegant way to do this but idec
    char prefix = (base == 2) ? 'b' : (base == 8) ? 'o' : (base == 16) ? 'x' : '\0';
    
    int i = bufSize - 1;
    int j = 0;

    while (n > 0) {
        buf[i] = "0123456789ABCDEF"[n % base];
        i--;
        n = n / base;
    }

    if (sign) {
        buf[j] = '-';
        j++;
    }

    if (prefix) {
        buf[j] = '0';
        buf[j + 1] = prefix;
        j += 2;
    }

    // Reverse buffer
    while(i++ < bufSize + 1) {
        buf[j++] = buf[i];
    }
    buf[j] = 0; // Terminate
}

int UIntToStr(char *buf, int bufSize, unsigned long long n, int base) {
    IntToStr(buf, bufSize, n, 0, base);
}

int SIntToStr(char *buf, int bufSize, long long n, int base) {
    IntToStr(buf, bufSize, (n < 0) ? -n : n, (n < 0), base); // arg 2 is absolute value
}

long long StrToInt(char *str, int base) {
    char *p = str;

    // Check for any prefixing
    int sign = 0;
    if (*p == '-') {
        sign = 1;
        p++;
    }

    // We cannot assume we can skip two chars here because the number could be little endian
    if (*p == '0') {
        char prefix[3] = {'b', 'o', 'x'};
        for (int i = 0; i < sizeof(prefix); i++) {
            if (*(p + 1) == prefix[i]) {
                p += 2;
                break;
            }
        }
    }

    long long result = 0;
    while (*p) {
        for (int i = 0; i < base; i++) {
            if (*p == "0123456789ABCDEF"[i]) {
                result = result * base + i;
                break;
            }
        }
        p++;
    }

    return result * (!sign ? 1 : -1);
}

int IsAlphaNumeric(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}
int IsWhiteSpace(char c) {
    return (c == ' ' || c == '\n');
}

char *NextLine(char *p) {
    while (*p++ != '\n');
    return p;
}

int TOMLCopySection(char *p, struct TOMLEntry *entry) {
    if (*p != '[') {
        return 0; // Make sure were in a TOML section
    }

    int i = 0;
    while (*++p != ']' && i < sizeof(entry->Section)) {
        if (*p == '\n') {
            break; // Done reading
        }
        if (!IsAlphaNumeric(*p)) {
            return 0; // Parse error
        }
        entry->Section[i] = *p;
        i++;
    }
    entry->Section[i] = '\0';

    return 1;
}

int TOMLCopyKey(char *p, struct TOMLEntry *entry) {
    int i = 0;
    while (*p != ' ') {
        if (!IsAlphaNumeric(*p)) {
            return 0; // Parsing error
        }
        entry->Key[i] = *p;
        i++;
        p++;
    }
    entry->Key[i] = '\0';

    return 1;
}

int TOMLDeserializeEntry(char *line, struct TOMLEntry *entry) {
    char *p = line;
    while (IsWhiteSpace(*p)) p++; // Skip whitespace

    if (*p == '[') {
        return 2; // Skip section headers
    }

    TOMLCopyKey(p, entry);

    return 1;
}

