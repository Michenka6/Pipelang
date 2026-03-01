#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include "nob.h"

typedef enum Token_Kind
{
    Token_Kind_INVALID,
    Token_Kind_INT,
    Token_Kind_FLOAT,
    Token_Kind_CHAR,
    Token_Kind_IDENT,
    Token_Kind_STRING_LITERAL,
    Token_Kind_EOF,
    Token_Kind_COUNT,
} Token_Kind;

typedef struct Token
{
    Nob_String_View payload;
    Token_Kind kind;
    size_t line;
    size_t column;
} Token;

typedef struct Tokens
{
    Token* items;
    size_t count;
    size_t capacity;
} Tokens;

#endif // TOKEN_H
