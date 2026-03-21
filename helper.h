/* Helper functions */ 
// TODO: Stop incrementing pointers, terrible idea
// The toml library is cleaner and safer than basically everything here, these implementations need work but get the job done
// The main library doesnt use these functions often and you can swap these out for something in stdlib or whatever

int IsNumeric(char c) {
    return (c >= '0' && c <= '9');
}

int IsNumericHex(char c) {
    return ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

int IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int IsAlphaNumeric(char c) {
    return IsAlpha(c) || IsNumeric(c);
}

int IsWhiteSpace(char c) {
    return (c == ' ' || c == '\n' || c == '\r');
}

int SkipWhitespace(char *string, int size) {
    int i = 0;
    while (i < size && string[i] != '\0') { 
        if (!IsWhiteSpace(string[i])) {
            break;
        }
        
        i++;
    };

    return i;
}

int StrCmp(char *source, int sourceSize, char *target, int targetSize) {
    int i = 0;
    if (targetSize > sourceSize) {
        return 0;
    }

    while (i < sourceSize && i < targetSize && !IsWhiteSpace(source[i]) && source[i] != '\0') {
        if (source[i] != target[i]) {
            return 0;
        } 
        i++;
    }

    return 1; // Does not ensure either string is terminated
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

// TODO: Error handling PLEASE
long long StrToInt(char *str, int bufSize, int base) {
    char *p = str;
    int i = 0;

    int sign = 0;
    if (*p == '-') {
        sign = 1;
        p++;
        i++;
    } else if (*p == '+') {
        p++;
        i++;
    }

    if (*p == '0') {
        if (*(p + 1) == 'b' || *(p + 1) == 'o' || *(p + 1) == 'x') {
            p += 2;
            i += 2;
        }
    }

    if (base < 2 || base > 16 || base % 2 != 0) {
        return 0; // Fail, unsupported base
    }

    long long result = 0;
    while (i++ < bufSize && *p != '\0') {
        for (int j = 0; j < base; j++) {
            if (ToUpper(*p) == "0123456789ABCDEF"[j]) {
                result = result * base + j;
                break;
            }
        }
        p++;
    }

    return result * (!sign ? 1 : -1);
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


double StrToFloat(char *buf, int bufSize)
{
    char *p = buf;
    int sign = 1;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    int i = 0;
    while (i < bufSize && IsNumeric(p[i])) {
        i++;
    }

    double before = (double)StrToInt(p, i, 10);
    p += i;

    if (*p != '.') {
        return sign * before;
    }
    p++;

    int j = 0;
    while (j < bufSize && IsNumeric(p[j])) {
        j++;
    }

    double after = (double)StrToInt(p, j, 10);
    double scale = 1.0;
    for (int k = 0; k < j; k++) {
        scale *= 10.0;
    }

    return sign * (before + after / scale);
}

int NextLine(char *buf, int size) {
    int i = 0;

    while (i < size) {
        if (buf[i] == '\n') {
            return i + 1;
        } else if (buf[i] == '\r') {
            if (i + 1 == '\n') {
                return i + 2;
            } else {
                return i + 1; // Might as well return, only using cr has technically existed
            }
        }
        i++;
    }

    return 0;
}

int MemCpy(char *source, int sourceSize, char *target, int targetSize) {
    int i = 0;

    if (sourceSize > targetSize) {
        return 0;
    }

    while (i < sourceSize) {
        target[i] = source[i];
        i++;
    }

    return 1;
}

