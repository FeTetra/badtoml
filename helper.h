#ifndef HELPER_H
#define HELPER_H

#include <string.h>
#include <ctype.h>

/* Helper functions */ 
/* 
    Many of these can be replaced with stdlib functions and more will be replaceable later.
    These use the pascal casing convention, to replace these with stdlib funcions you will 
    need to rename any calls of these to the standard function names.
    Example: StrLen(someStr); -> strlen(someStr);

    Exceptions: StrToLL() is not compatible with strtoll()
*/

static inline unsigned int Skip(char *string, size_t size) {
    size_t i = 0;

    while (i < size && string[i] != '\0') { 
        if (!isspace(string[i])) {
            break;
        }
        
        i++;
    };

    return i;
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

static inline void ULLToStr(char *buf, int bufSize, unsigned long long n, int base) {
    IntToStr(buf, bufSize, n, 0, base);
}

static inline void SLLToStr(char *buf, int bufSize, long long n, int base) {
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
            if (toupper(*p) == "0123456789ABCDEF"[j]) {
                result = result * base + j;
                break;
            }
        }
        p++;
    }

    return result * (!sign ? 1 : -1);
}

static inline int DoubleToStr(char *buf, int bufSize, float n, int round) {
    char *p = buf;

    int sign = 0;

    long long iPart = (long long)n;
    IntToStr(p, bufSize, iPart, (n < 0), 10); // Always use base 10?
    while (isdigit(*++p)); // Skip to end of added data
    double fPart = n - (double)iPart;

    if (round != 0) {
        *p++ = '.';
        fPart = fPart * PowerD(10, round);
        // Just realized that passing bufSize like this is really stupid, fix later
        ULLToStr(p, bufSize, (long long)fPart, 10); // TODO: actually write slightly safe code
    }

    return 1;
}


static inline double StrToDouble(char *buf, int bufSize)
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
    while (i < bufSize && isdigit(p[i])) {
        i++;
    }

    double before = (double)StrToLL(p, i, 10);
    p += i;

    if (*p != '.') {
        return sign * before;
    }
    p++;

    int j = 0;
    while (j < bufSize && isdigit(p[j])) {
        j++;
    }

    double after = (double)StrToLL(p, j, 10);
    double scale = 1.0;
    for (int k = 0; k < j; k++) {
        scale *= 10.0;
    }

    return sign * (before + after / scale);
}

static inline unsigned int NextLine(char *buf, size_t size) {
    for (size_t i = 0; i <= size; i++) {
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

#endif
