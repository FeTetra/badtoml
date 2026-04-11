#include "helper.h"
#include <stddef.h>
#include <string.h>

typedef enum TokenType {
    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_KEY,
    TOKEN_SECTION,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_DATETIME,
    TOKEN_BOOL,

    TOKEN_EQUALS,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,

    TOKEN_NEWLINE
} TokenType;

typedef struct Token {
    TokenType type;
    const char *start;
    size_t len;
    unsigned int line;
    const char *error;
} Token;

typedef struct Lexer {
    const char *input;
    size_t pos;
    size_t len;
    unsigned int line;
} Lexer;

/* Low-level lexer functions */

int IsAtEnd(Lexer *l) {
    return (l->pos >= l->len);
}

char Peek(Lexer *l) {
    if (IsAtEnd(l)) return '\0';
    return l->input[l->pos];
}

char PeekNext(Lexer *l) {
    if (l->pos + 1 >= l->len) return '\0';
    return l->input[l->pos + 1];
}

char Advance(Lexer *l) {
    char result = Peek(l);

    l->pos++;
    if (result == '\n') {
        l->line++;
    } else if (result == '\r') {
        l->pos += (1 + (PeekNext(l) == '\n'));
        l->line++;
    }

    return result;
}

int Match(Lexer *l, char expected) {
    if (Peek(l) != expected) return 0;
    l->pos++;
    return 1;
}

void SkipWhiteSpace(Lexer *l) {
    for (;;) {
        char c = Peek(l);
        if (IsSpace(c)) {
            Advance(l);
        } else if (c == '#') {
            while (Peek(l) != '\n' && Peek(l) != '\r' && !IsAtEnd(l)) Advance(l);
        } else {
            break;
        }
    }
}

/* Token constructors */

Token MakeToken(Lexer *l, TokenType type, const char *start) {
    Token t = {
        .type = type,
        .start = start,
        .len = (size_t)(l->input + l->pos - start),
        .line = l->line,
        .error = NULL,
    };

    return t;
}

Token ErrorToken(Lexer *l, const char *msg) {
    Token t = {
        .type = TOKEN_ERROR,
        .start = NULL,
        .len = 0,
        .line = l->line,
        .error = msg,
    };

    return t;
}

/* Tokenization */

Token ScanString(Lexer *l, char quote) {
    const char *start = l->input + l->pos - 1;

    int multiline = 0;
    if (Peek(l) == quote && PeekNext(l) == quote) {
        multiline = 1;
        Advance(l);
        Advance(l);
    }

    while (!IsAtEnd(l)) {
        char c = Advance(l);

        if (!multiline && c == '\\' && quote == '\"') {
            Advance(l);
            continue;
        }

        if (c == quote) {
            if (multiline) {
                if (Peek(l) == quote && PeekNext(l) == quote) {
                    Advance(l);
                    Advance(l);
                    return MakeToken(l, TOKEN_STRING, start);
                }
            } else {
                return MakeToken(l, TOKEN_STRING, start);
            }
        }
    }

    return ErrorToken(l, "Unterminated string");
}

Token ScanNumberOrDatetime(Lexer *l) {
    const char *start = l->input + l->pos -1;
    int hasDash = 0, hasColon = 0;

    if (Peek(l) == '+') {
        Advance(l);
    }

    while (IsDigit(Peek(l)) || Peek(l) == '-' || Peek(l) == 'T' 
        || Peek(l) == 'Z' || Peek(l) == '.') {
        char c = Advance(l);
        if (c == '-') hasDash = 1;
        if (c == ':') hasColon = 1;
    }

    if (hasDash && hasColon) {
        return MakeToken(l, TOKEN_DATETIME, start);
    }

    return MakeToken(l, TOKEN_NUMBER, start);
}

Token ScanIdentifier(Lexer *l) {
    const char *start = l->input + l->pos - 1;

    while (IsAlNum(Peek(l)) || Peek(l) == '_' || Peek(l) == '-') {
        Advance(l);
    }

    size_t len = l->input + l->pos - start;

    if (len == 4 && StrNCmp(start, "true", 4) == 0)
        return MakeToken(l, TOKEN_BOOL, start);
    if (len == 5 && StrNCmp(start, "false", 5) == 0)
        return MakeToken(l, TOKEN_BOOL, start);

    return MakeToken(l, TOKEN_KEY, start);
}

Token NextToken(Lexer *l) {
    SkipWhiteSpace(l);

    if (IsAtEnd(l)) {
        return MakeToken(l, TOKEN_EOF, l->input + l->pos);
    }

    const char *start = l->input + l->pos;
    char c = Advance(l);

    switch (c) {
        case '\n': return MakeToken(l, TOKEN_NEWLINE, start);
        case '=': return MakeToken(l, TOKEN_EQUALS, start);
        case '.': return MakeToken(l, TOKEN_DOT, start);
        case ',': return MakeToken(l, TOKEN_COMMA, start);
        case '[': return MakeToken(l, TOKEN_LBRACKET, start);
        case ']': return MakeToken(l, TOKEN_RBRACKET, start);

        case '\"':
        case '\'':
            return ScanString(l, c);
    }

    if (IsDigit(c) || (((c == '-') || (c == '+')) && IsDigit(Peek(l)))) {
        return ScanNumberOrDatetime(l);
    }

    if (IsAlpha(c) || c == '_' || c == '-') {
        return ScanIdentifier(l);
    }

    return  ErrorToken(l, "Unexpected character");
}
