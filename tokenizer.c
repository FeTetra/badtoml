#include "tokenizer.h"

/* Low-level lexer functions */

int IsAtEnd(Lexer *l) {
    return *l->current == '\0';
}

char Advance(Lexer *l) {
    return *l->current++;
}

char Peek(Lexer *l) {
    return *l->current;
}

char PeekNext(Lexer *l) {
    if (IsAtEnd(l)) return '\0';
    return l->current[1];
}

int IsAlphaOrSeparator(char c) {
    return isalpha(c) || c == '_' || c == '-';
}

/* Constructors */

Lexer MakeLexer(char *buf) {
    Lexer l;
    l.start = buf;
    l.current = buf;
    l.line = 0;
    return l;
}

Token MakeToken(Lexer *l, unsigned int type) {
    Token t;
    t.type = type;
    t.intType = TOKEN_INT_NONE;
    t.start = l->start;
    t.length = (size_t)(l->current - l->start);
    t.line = l->line;
    return t;
}

Token MakeTokenInt(Lexer *l, unsigned int intType) {
    Token t;
    t.type = TOKEN_INT;
    t.intType = intType;
    t.start = l->start;
    t.length = (size_t)(l->current - l->start);
    t.line = l->line;
    return t;
}

Token ErrorToken(Lexer *l, const char *msg) {
    Token t;
    t.type = TOKEN_ERROR;
    t.start = msg;
    t.length = strlen(msg);
    t.line = l->line;
    return t;
}

/* Tokenization */

void SkipWhitespace(Lexer *l) {
    for (;;) {
        char c = Peek(l);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                Advance(l);
                break;

            case '#':
                // Comment
                while (Peek(l) != '\n' && !IsAtEnd(l)) Advance(l);
                Advance(l); // Skip this newline too
                break;

            default:
                return;
        }
    }
}

Token ScanString(Lexer *l, char quote) {
    while (Peek(l) != quote && !IsAtEnd(l)) {
        if (Peek(l) == '\n') l->line++;
        Advance(l);
    }

    if (IsAtEnd(l)) {
        return ErrorToken(l, "Unterminated string");
    }

    Advance(l);
    if (quote == '\'') return MakeToken(l, TOKEN_STRING_LITERAL);
    else return MakeToken(l, TOKEN_STRING);
}

Token ScanIdentifier(Lexer *l) {
    while (IsAlphaOrSeparator(Peek(l)) || isdigit(Peek(l))) {
        Advance(l);
    }

    size_t length = l->current - l->start;

    if (length == 4 && strncmp(l->start, "true", 4) == 0)
        return MakeToken(l, TOKEN_TRUE);
    if (length == 5 && strncmp(l->start, "false", 5) == 0)
        return MakeToken(l, TOKEN_FALSE);

    return MakeToken(l, TOKEN_IDENTIFIER);
}

Token ScanNumberOrDate(Lexer *l) {
    int isFloat = 0;
    int isDate = 0;
    int hasSign = 0;

    int intType = TOKEN_INT_DEC;

    if (Peek(l) == '-' || Peek(l) == '+') {
        hasSign = 1;
        Advance(l);
    }

    if (Peek(l) == '0') {
        Advance(l);
        switch (Peek(l)) {
            case 'b':
                intType = TOKEN_INT_BIN;
                Advance(l);
                break;
            case 'o':
                intType = TOKEN_INT_OCT;
                Advance(l);
                break;
            case 'x':
                intType = TOKEN_INT_HEX;
                Advance(l);
                break;
        }
    }

    while (isdigit(Peek(l)) || (isxdigit(Peek(l)) && intType == TOKEN_INT_HEX)) Advance(l);

    // float
    if (Peek(l) == '.' && isdigit(PeekNext(l))) {
        if (intType != TOKEN_INT_DEC) return ErrorToken(l, "Float must be base 10");
        isFloat = 1;
        Advance(l);
        while (isdigit(Peek(l))) Advance(l);
    }

    if (Peek(l) == '-') {
        isDate = 1;
        while (isalnum(Peek(l)) || strchr("-:TZ", Peek(l))) {
            Advance(l);
        }
    }

    if (isDate) return MakeToken(l, TOKEN_DATE);
    if (isFloat) return MakeToken(l, TOKEN_FLOAT);
    return MakeTokenInt(l, intType);
}

Token NextToken(Lexer *l) {
    SkipWhitespace(l);

    l->start = l->current;

    if (IsAtEnd(l)) return MakeToken(l, TOKEN_EOF);

    char c = Peek(l);

    if (isalpha(c)) return ScanIdentifier(l);
    if (isdigit(c) || c == '-' || c == '+') return ScanNumberOrDate(l);

    c = Advance(l);

    switch (c) {
        case '[': return MakeToken(l, TOKEN_LBRACKET);
        case ']': return MakeToken(l, TOKEN_RBRACKET);
        case '{': return MakeToken(l, TOKEN_LBRACE);
        case '}': return MakeToken(l, TOKEN_RBRACE);
        case '=': return MakeToken(l, TOKEN_EQUAL);
        case ',': return MakeToken(l, TOKEN_COMMA);

        case '\'':
        case '\"':
            return ScanString(l, c);

        case '\n':
            l->line++;
            return MakeToken(l, TOKEN_NEWLINE);
    }

    return ErrorToken(l, "Unexpected character");
}
