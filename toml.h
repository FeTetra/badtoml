/* Type definitions */

struct TOMLEntry {
    char Section[32];
    char Key[32];
    union {
        char StrVal[64];
        int BoolVal;
        long long IntVal;
    } Value;
};

/* Helper functions */

int IsNumeric(char c) {
    return (c >= '-' && c <= '9');
}

int IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int IsAlphaNumeric(char c) {
    return IsAlpha(c) || IsNumeric(c);
}
int IsASCIISymbol(char c) {
    return (c >= ' ' && c <= '~');
}
int IsWhiteSpace(char c) {
    return (c == ' ' || c == '\n');
}

char ToUpper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32; // Distance between upper and lowercase variant in ASCII table
    }
    return c;
}

double PowerD (double x, int y)
{
    double temp;
    if (y == 0)
    return 1;
    temp = PowerD (x, y / 2);
    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0)
            return x * temp * temp;
        else
            return (temp * temp) / x;
    }
}

void NextLine(char **p) {
    while (**p != '\0' && **p != '\n') {
        (*p)++;
    }
    if (**p == '\n') {
        (*p)++;
    }
}

int StrLen(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

int StrContains(char *target, char *contains) {
    int i = 0;
    while (target[i] == contains[i] && contains[i] != '\0') {
        i++;
    }
    return (i == StrLen(contains));
}

/* Type (de)serialization */

// This works exclusively with integral values, as it takes advantage of integer arithmetic
// Note: at the moment this only outputs big endian
int IntToStr(char *buf, int bufSize, unsigned long long n, int sign, int base) {
    if (base < 2 || base > 16 || base % 2 != 0) {
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
    IntToStr(buf, bufSize, (n < 0) ? -n : n, (n < 0), base); // arg 3 is absolute value
}

int FloatToStr(char *buf, int bufSize, float n, int round) {
    char *p = buf;

    int sign = 0;

    long long iPart = (long long)n;
    IntToStr(p, bufSize, iPart, (n < 0), 10); // Always use base 10?
    while (IsNumeric(*++p)); // Skip to end of added data
    double fPart = n - (double)iPart;

    if (round != 0) {
        *p++ = '.';
        fPart = fPart * PowerD(10, round);
        // Just realized that passing bufSize like this is really stupid, fix later
        UIntToStr(p, bufSize, (long long)fPart, 10); // TODO: actually write slightly safe code
    }

    return 1;
}

// TODO: Error handling PLEASE
long long StrToInt(char *str, int sign, int base) {
    char *p = str;

    if (base < 2 || base > 16 || base % 2 != 0) {
        return 0; // Fail, unsupported base
    }

    long long result = 0;
    while (*p) {
        for (int i = 0; i < base; i++) {
            if (ToUpper(*p) == "0123456789ABCDEF"[i]) {
                result = result * base + i;
                break;
            }
        }
        p++;
    }

    return result * (!sign ? 1 : -1);
}

/* TOML Parsing */

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
        *p++;
    }
    entry->Key[i] = '\0';

    return 1;
}

int TOMLParseValue(char *p, struct TOMLEntry *entry) {
    while (IsWhiteSpace(*p)) p++; // Skip whitespace
    if (*p != '=') {
        return 0; // Not a value
    }
    while (IsWhiteSpace(*++p)); // More whitespace 

    // Parse double quote string
    if (*p == '\"') {
        p++;
        int i = 0;
        while (IsASCIISymbol(p[i]) && i < sizeof(entry->Value.StrVal) - 1 && p[i] != '\"') {
            entry->Value.StrVal[i] = p[i];
            i++;
        }
        return 1; // Might want some error handling
    }

    // Parse bool
    if (StrContains(p, "true")) {
        entry->Value.BoolVal = 1;
        return 1;
    } else if (StrContains(p, "false")) {
        entry->Value.BoolVal = 0;
        return 1;
    }

    // Parse int
    int hasSign = 0;
    if (*p == '-') {
        hasSign = 1;
        p++;
    }
    if (StrContains(p, "0b")) {
        p += 2;
        entry->Value.IntVal = StrToInt(p, hasSign, 2); // Binary
        return 1;
    }
    if (StrContains(p, "0o")) {
        p += 2;
        entry->Value.IntVal = StrToInt(p, hasSign, 8); // Octal
        return 1;
    }
    if (StrContains(p, "0x")) {
        p += 2;
        entry->Value.IntVal = StrToInt(p, hasSign, 16); // Hexadecimal
        return 1;
    }
    entry->Value.IntVal = StrToInt(p, hasSign, 10); // Decimal
    return 1; // Just assume its a base 10 int and return, other types have yet to be implemented
}

int TOMLDeserializeEntry(char *line, struct TOMLEntry *entry) {
    char *p = line;
    while (IsWhiteSpace(*p)) p++; // Skip whitespace
    if (*p == '[') {
        return 2; // Skip section headers
    }

    if (!TOMLCopyKey(p, entry)) {
        return 0;
    }
    while (IsAlphaNumeric(*p++));
    if (!TOMLParseValue(p, entry)) {
        return 0;
    }

    return 1;
}

