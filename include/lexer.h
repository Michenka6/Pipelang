#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include "nob.h"
#include "token.h"

typedef struct Lexer
{
    Nob_String_View source;
    size_t cursor;
    size_t line;
    size_t column;
} Lexer;

bool lexer_get_next_token(Lexer *l, Token *out);
void lexer_tokenize(Lexer *lexer, Tokens *tokens);

#endif // LEXER_H
