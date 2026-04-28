#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

typedef enum {
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
} TokenType;

typedef enum {
    TOKEN_INT_NONE = 0,
    TOKEN_INT_BIN = 2,
    TOKEN_INT_OCT = 8,
    TOKEN_INT_DEC = 10,
    TOKEN_INT_HEX = 16,
} TokenIntType;

typedef struct {
    TokenType type;
    TokenIntType intType;
    const char *start;
    size_t length;
    int line;
} Token;

typedef struct {
    const char *start;
    const char *current;
    int line;
} Lexer;

/* Low-level lexer functions */

int IsAtEnd(Lexer *l);

char Peek(Lexer *l);

char PeekNext(Lexer *l);

char Advance(Lexer *l);

void SkipWhiteSpace(Lexer *l);

/* struct Lexer constructors */
Lexer MakeLexer(char *buf);

/* struct Token constructors */

Token MakeToken(Lexer *l, TokenType type);

Token ErrorToken(Lexer *l, const char *msg);

/* Tokenization */

Token ScanString(Lexer *l, char quote);

Token ScanNumberOrDatetime(Lexer *l);

Token ScanIdentifier(Lexer *l);

Token NextToken(Lexer *l);

#endif //TOKENIZER_H
