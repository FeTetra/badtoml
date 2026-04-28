#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "helper.h"

enum TokenType{
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_EQUAL,
    TOKEN_COMMA,

    TOKEN_IDENTIFIER,
    TOKEN_STRING_LITERAL,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_DATE,

    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_ERROR
};

enum TokenIntType {
    TOKEN_INT_NONE = 0,
    TOKEN_INT_BIN = 2,
    TOKEN_INT_OCT = 8,
    TOKEN_INT_DEC = 10,
    TOKEN_INT_HEX = 16,
};

struct Token {
    unsigned int type;
    unsigned int intType;
    const char *start;
    int length;
    int line;
};

struct Lexer {
    const char *start;
    const char *current;
    int line;
};

/* Low-level lexer functions */

int IsAtEnd(struct Lexer *l);

char Peek(struct Lexer *l);

char PeekNext(struct Lexer *l);

char Advance(struct Lexer *l);

void SkipWhiteSpace(struct Lexer *l);

/* struct Lexer constructors */
struct Lexer MakeLexer(char *buf);

/* struct Token constructors */

struct Token MakeToken(struct Lexer *l, unsigned int type);

struct Token ErrorToken(struct Lexer *l, const char *msg);

/* Tokenization */

struct Token ScanString(struct Lexer *l, char quote);

struct Token ScanNumberOrDatetime(struct Lexer *l);

struct Token ScanIdentifier(struct Lexer *l);

struct Token NextToken(struct Lexer *l);

#endif //TOKENIZER_H
