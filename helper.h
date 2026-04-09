#ifndef HELPER_H
#define HELPER_H

/* Helper functions */ 
// TODO: Stop incrementing pointers, terrible idea
// The toml library is cleaner and safer than basically everything here, these implementations need work but get the job done
// The main library doesnt use these functions often and you can swap these out for something in stdlib or whatever

#define NULL ((void *)0)

static inline int IsDigit(int ch) {
    return (ch >= '0' && ch <= '9');
}

static inline int IsXDigit(int ch) {
    return (IsDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'));
}

static inline int IsUpper(int ch) {
    return (ch >= 'A' && ch <= 'Z');
}

static inline int IsLower(int ch) {
    return (ch >= 'a' && ch <= 'z');
}

static inline int IsAlpha(int ch) {
    return (IsUpper(ch) || IsLower(ch));
}

static inline int IsAlphaNum(int ch) {
    return (IsAlpha(ch) || IsDigit(ch));
}

static inline int IsSpace(int ch) {
    return (ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r' || ch == ' ');
}

static inline char ToUpper(int ch) {
    if (IsLower(ch)) {
        return ch - 32;
    }

    return ch;
}

static inline int SkipWhitespace(char *string, int size) {
    int i = 0;
    while (i < size && string[i] != '\0') { 
        if (!IsSpace(string[i])) {
            break;
        }
        
        i++;
    };

    return i;
}

static inline int StrCmp(char *source, int sourceSize, char *target, int targetSize) {
    int i = 0;
    if (targetSize > sourceSize) {
        return 0;
    }

    while (i < sourceSize && i < targetSize) {
        if (source[i] != target[i]) {
            return 0;
        } 
        i++;
    }

    return 1; // Does not ensure either string is terminated
}

static inline int StrLen(char *str) {
    int i = 0;

    while (str[i] != '\0') { i++; };

    return i; // Gross
}

static inline double PowerD (double x, int y)
{
    double temp;
    if (y == 0) { return 1; }
    temp = PowerD(x, y / 2);
    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0)
            return x * temp * temp;
        else
            return (temp * temp) / x;
    }
}

static inline int IntToStr(char *buf, int bufSize, unsigned long long n, int sign, int base) {
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

static inline void UIntToStr(char *buf, int bufSize, unsigned long long n, int base) {
    IntToStr(buf, bufSize, n, 0, base);
}

static inline void SIntToStr(char *buf, int bufSize, long long n, int base) {
    IntToStr(buf, bufSize, (n < 0) ? -n : n, (n < 0), base); // arg 3 is absolute value
}

// Not quite the same as most implementations, may never change
static inline long long StrToLL(char *str, int bufSize, int base) {
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

static inline int FloatToStr(char *buf, int bufSize, float n, int round) {
    char *p = buf;

    int sign = 0;

    long long iPart = (long long)n;
    IntToStr(p, bufSize, iPart, (n < 0), 10); // Always use base 10?
    while (IsDigit(*++p)); // Skip to end of added data
    double fPart = n - (double)iPart;

    if (round != 0) {
        *p++ = '.';
        fPart = fPart * PowerD(10, round);
        // Just realized that passing bufSize like this is really stupid, fix later
        UIntToStr(p, bufSize, (long long)fPart, 10); // TODO: actually write slightly safe code
    }

    return 1;
}


static inline double StrToFloat(char *buf, int bufSize)
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
    while (i < bufSize && IsDigit(p[i])) {
        i++;
    }

    double before = (double)StrToLL(p, i, 10);
    p += i;

    if (*p != '.') {
        return sign * before;
    }
    p++;

    int j = 0;
    while (j < bufSize && IsDigit(p[j])) {
        j++;
    }

    double after = (double)StrToLL(p, j, 10);
    double scale = 1.0;
    for (int k = 0; k < j; k++) {
        scale *= 10.0;
    }

    return sign * (before + after / scale);
}

static inline unsigned int NextLine(char *buf, unsigned int size) {
    for (unsigned int i = 0; i <= size; i++) {
        if (buf[i] == '\n') {
            return i + 1;
        } else if (buf[i] == '\r') {
            if (i + 1 == '\n') {
                return i + 2;
            } else {
                return i + 1; // Might as well return, only using cr has technically existed
            }
        } else if (buf[i] == '\0') {
            return i;
        }
    }

    return 0;
}

static inline void *MemCpy(void *dest, const void *src, unsigned int count) {
    char *d = (char*)dest;
    const char *s = (char*)src;

    for (unsigned int i = 0; i < count; i++ ) {
        d[i] = s[i];
    }

    return dest;
}

static inline void *MemSet(void *dest, int ch, unsigned int count) {
    char *d = (char *)dest;

    for (unsigned int i = 0; i < count; i++) {
        d[i] = (unsigned char)ch;
    }

    return dest;
}

#endif
