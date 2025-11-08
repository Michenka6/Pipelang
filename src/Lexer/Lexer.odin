package Lexer

import "core:fmt"

TAB_LENGTH :: 4

Lexer :: struct {
    data:   []u8,
    cursor: int,
    token:  Token,
}

Token_Kind :: enum {
    INVALID,
    INT,
    FLOAT,
    CHAR,
    IDENT,
    STRING_LITERAL,
}

Token :: struct {
    kind:    Token_Kind,
    payload: string,
}

tk_to_string :: proc(tk: Token) -> string {
    switch tk.kind {
    case .INVALID:
        return "INVALID"
    case .CHAR:  fallthrough
    case .INT:   fallthrough
    case .FLOAT: fallthrough
    case .IDENT:
        return fmt.tprintf(" %s ", tk.payload)
    case .STRING_LITERAL:
        return fmt.tprintf("\"%s\"", tk.payload)
    }
    return "UNEXPECTED END";
}

print_tokens :: proc(tks: []Token) {
    for tk, i in tks {
        fmt.printf("%v : %v\n", i, tk_to_string(tk))
    }
    fmt.println()
}

lexer_make :: proc(data: []u8) -> (lexer: Lexer) {
    lexer.data   = data
    lexer.cursor = 0
    return
}

lexer_tokenize :: proc(lexer: ^Lexer, tokens: ^[dynamic]Token) {
    tk, ok := lexer_get_next_token(lexer)
    for ok {
        append_elem(tokens, tk)
        tk, ok = lexer_get_next_token(lexer)
    }
}

lexer_get_next_token :: proc(lexer: ^Lexer) -> (tk: Token, ok: bool) {
    skip_whitespace(lexer)
    c := peek(lexer) or_return

    if c == '-' {
        backtrack := lexer.cursor
        lexer.cursor += 1
        c = peek(lexer) or_return
        if c == '-' {
            lexer.cursor += 1
            c = peek(lexer) or_return
            for c != '\n' {
                lexer.cursor += 1
                c = peek(lexer) or_return
            }
            lexer.cursor += 1
        } else {
            lexer.cursor = backtrack
        }
    }
    skip_whitespace(lexer)
    c = peek(lexer) or_return

    if is_digit(c) {
        tk.kind    = .INT

        start := lexer.cursor
        end   := start
        for is_digit(c) {
            lexer.cursor += 1
            end += 1
            c = peek(lexer) or_return
            if c == '.' {
                lexer.cursor += 1
                end += 1
                tk.kind = .FLOAT
                c = peek(lexer) or_return
            }
        }
        tk.payload = string(lexer.data[start:end])
        return tk, true
    }

    if c == '"' {
        tk.kind = .STRING_LITERAL
        start := lexer.cursor
        end   := start

        end += 1
        lexer.cursor += 1
        c = peek(lexer) or_return

        for c != '"' {
            end += 1
            lexer.cursor += 1
            c = peek(lexer) or_return
        }
        end += 1
        lexer.cursor += 1
        tk.payload = string(lexer.data[start+1:end-1])
        return tk, true
    }

    if is_letter(c) {
        tk.kind = .IDENT
        start := lexer.cursor
        end   := start
        for is_letter(c) || c == '_' || is_digit(c) {
            end += 1
            lexer.cursor += 1
            c = peek(lexer) or_return
        }
        tk.payload = string(lexer.data[start:end])
        return tk, true
    }

    tk.kind = .CHAR
    tk.payload = string(lexer.data[lexer.cursor:lexer.cursor+1])
    lexer.cursor += 1
    return tk, true
}

is_digit :: proc(c: u8) -> bool {
    return '0' <= c && c <= '9'
}

is_letter :: proc(c: u8) -> bool {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')
}

skip_whitespace :: proc(lexer: ^Lexer) {
    c, ok := peek(lexer)

    for ok {
        switch c {
        case ' ': 
        case '\r':
        case '\n':
        case '\t':
        case:      return
        }
        lexer.cursor += 1
        c, ok = peek(lexer)
    }
}

peek :: proc(lexer: ^Lexer) -> (c: u8, ok: bool) {
    assert(0 <= lexer.cursor)
    if lexer.cursor < len(lexer.data) {
        return lexer.data[lexer.cursor], true
    }
    return 0x69, false
}

