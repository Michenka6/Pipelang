#include "../include/lexer.h"

#include <ctype.h>

static inline bool
lexer_peek(Lexer* l, char* out)
{
    if (l->cursor >= l->source.count) return false;
    *out = l->source.data[l->cursor];
    return true;
}

static inline bool
lexer_advance(Lexer* l, char* out)
{
    if (!lexer_peek(l, out)) return false;

    if (*out == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }

    l->cursor++;
    return true;
}

static void
lexer_skip_whitespace(Lexer* l)
{
    char c;
    while (lexer_peek(l, &c) && isspace((unsigned char)c)) {
        lexer_advance(l, &c);
    }
}

bool
lexer_get_next_token(Lexer* l, Token* out)
{
    lexer_skip_whitespace(l);

    char c;
    if (!lexer_peek(l, &c)) {
        out->kind = Token_Kind_EOF;
        out->payload = (Nob_String_View){0};
        out->line = l->line;
        out->column = l->column;
        return true;
    }

    size_t tok_line = l->line;
    size_t tok_col  = l->column;

    if (c == '-') {
        size_t save_cursor = l->cursor;
        size_t save_line   = l->line;
        size_t save_col    = l->column;

        lexer_advance(l, &c);
        if (lexer_peek(l, &c) && c == '-') {
            lexer_advance(l, &c);
            while (lexer_peek(l, &c) && c != '\n') {
                lexer_advance(l, &c);
            }
            if (lexer_peek(l, &c) && c == '\n') {
                lexer_advance(l, &c);
            }
            return lexer_get_next_token(l, out);
        }

        l->cursor = save_cursor;
        l->line   = save_line;
        l->column = save_col;
    }

    if (isdigit((unsigned char)c)) {
        size_t start = l->cursor;
        bool seen_dot = false;

        while (lexer_peek(l, &c)) {
            if (isdigit((unsigned char)c)) {
                lexer_advance(l, &c);
            } else if (!seen_dot && c == '.') {
                seen_dot = true;
                lexer_advance(l, &c);
            } else {
                break;
            }
        }

        out->kind = seen_dot ? Token_Kind_FLOAT : Token_Kind_INT;
        out->payload = nob_sv_from_parts(l->source.data + start, l->cursor - start);
        out->line = tok_line;
        out->column = tok_col;
        return true;
    }

    if (c == '"') {
        lexer_advance(l, &c);
        size_t start = l->cursor;

        while (lexer_peek(l, &c) && c != '"') {
            lexer_advance(l, &c);
        }

        if (!lexer_peek(l, &c)) {
            out->kind = Token_Kind_INVALID;
            out->line = tok_line;
            out->column = tok_col;
            return true;
        }

        out->kind = Token_Kind_STRING_LITERAL;
        out->payload = nob_sv_from_parts(l->source.data + start, l->cursor - start);
        out->line = tok_line;
        out->column = tok_col;

        lexer_advance(l, &c);
        return true;
    }

    if (isalpha((unsigned char)c) || c == '_') {
        size_t start = l->cursor;
        do {
            lexer_advance(l, &c);
        } while (lexer_peek(l, &c) && (isalnum((unsigned char)c) || c == '_'));

        out->kind = Token_Kind_IDENT;
        out->payload = nob_sv_from_parts(l->source.data + start, l->cursor - start);
        out->line = tok_line;
        out->column = tok_col;
        return true;
    }

    out->kind = Token_Kind_CHAR;
    out->payload = nob_sv_from_parts(l->source.data + l->cursor, 1);
    out->line = tok_line;
    out->column = tok_col;

    lexer_advance(l, &c);
    return true;
}

void
lexer_tokenize(Lexer* lexer, Tokens* tokens)
{
    Token token;
    while (lexer_get_next_token(lexer, &token)) {
        nob_da_append(tokens, token);
        if (token.kind == Token_Kind_EOF) break;
    }
}
