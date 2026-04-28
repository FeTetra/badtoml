#ifndef HELPER_H
#define HELPER_H

/* Helper functions */ 
/* 
    Many of these can be replaced with stdlib functions and more will be replaceable later.
    These use the pascal casing convention, to replace these with stdlib funcions you will 
    need to rename any calls of these to the standard function names.
    Example: StrLen(someStr); -> strlen(someStr);

    Exceptions: StrToInt() is not compatible with strtoll()
*/

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

static inline int IsAlNum(int ch) {
    return (IsAlpha(ch) || IsDigit(ch));
}

static inline int IsSpace(int ch) {
    return (ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r' || ch == ' ');
}

static inline int ToUpper(int ch) {
    if (IsLower(ch)) {
        return ch - 32;
    }

    return ch;
}

static inline unsigned int StrLen(const char *str) {
    unsigned int i = 0;

    while (str[i] != '\0') { i++; }

    return i;
}

static inline char *StrChr(char *str, int ch) {
    ch = (unsigned char)ch;
    unsigned int len = StrLen(str);
    unsigned int i = 0;

    while (i < len) {
        if (str[i] == ch) return &str[i];
        i++;
    }

    return (char *)NULL;
}

static inline int StrNCmp(const char *lhs, const char *rhs, unsigned int count) {
    unsigned int i = 0;

    while (i < count && lhs[i] && (lhs[i] == rhs[i])) { i++; }

    if (i == count) {
        return 0; // Apparently a strncmp edge case
    }

    return (const unsigned char)lhs[i] - (const unsigned char)rhs[i];
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
    
    return 1;
}

static inline void UIntToStr(char *buf, int bufSize, unsigned long long n, int base) {
    IntToStr(buf, bufSize, n, 0, base);
}

static inline void SIntToStr(char *buf, int bufSize, long long n, int base) {
    IntToStr(buf, bufSize, (n < 0) ? -n : n, (n < 0), base); // arg 3 is absolute value
}

// Not quite the same as most implementations, may never change
static inline long long StrToInt(const char *str, int bufSize, int base) {
    const char *p = str;
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

    if (n < 0) {
        n *= -1;
        *p++ = '-';
    }

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


static inline double StrToFloat(const char *buf, int bufSize)
{
    const char *p = buf;
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

    double before = (double)StrToInt(p, i, 10);
    p += i;

    if (*p != '.') {
        return sign * before;
    }
    p++;

    int j = 0;
    while (j < bufSize && IsDigit(p[j])) {
        j++;
    }

    double after = (double)StrToInt(p, j, 10);
    double scale = 1.0;
    for (int k = 0; k < j; k++) {
        scale *= 10.0;
    }

    return sign * (before + after / scale);
}

#endif
