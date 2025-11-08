package Parser

import "../Lexer"
import "../AST"
import "core:fmt"
import "core:strconv"
import "core:strings"
import "core:os"

Parser :: struct {
    tokens: []Lexer.Token,
    cursor: int,
}

eat_tk :: proc(p: ^Parser, kind: Lexer.Token_Kind, payload: string = "") -> (ok: bool) {
    if p.cursor >= len(p.tokens) { return }
    tk := p.tokens[p.cursor]
    if tk.kind == kind && (tk.payload == payload || payload == "") {
        p.cursor += 1
        return true
    }
    return
}

eat_tk_and_return :: proc(p: ^Parser, kind: Lexer.Token_Kind, payload: string = "") -> (load: string, ok: bool) {
    if p.cursor >= len(p.tokens) { return }
    tk := p.tokens[p.cursor]
    if tk.kind == kind && (tk.payload == payload || payload == ""){
        p.cursor += 1
        return tk.payload, true
    }
    return
}

parse_program :: proc(p: ^Parser) -> (program: ^AST.Program, ok: bool) {
    blocks := parse_many_blocks(p) or_return

    program = new(AST.Program)
    program^ = AST.Program { blocks }
    return program, true
}

parse_many_blocks :: proc(p: ^Parser) -> ([]AST.Block, bool) {
    block, kk := parse_block(p)

    blocks := make([dynamic]AST.Block)
    for kk {
        append_elem(&blocks, block^)
        block, kk = parse_block(p)
    }
    return blocks[:], true
}

parse_block :: proc(p: ^Parser) -> (program: ^AST.Block, ok: bool) {
    if eat_tk(p, .CHAR, "[") {
        blocks := parse_many_blocks(p) or_return
        eat_tk(p, .CHAR, "]") or_return
        program = new(AST.Block)
        program^ = AST.Sequence { blocks }
        return program, true
    }

    if eat_tk(p, .CHAR, "{") {
        blocks := parse_many_blocks(p) or_return
        eat_tk(p, .CHAR, "|") or_return
        folder := eat_tk_and_return(p, .IDENT) or_return
        eat_tk(p, .CHAR, "}") or_return
        program = new(AST.Block)
        program^ = AST.Branch { folder, blocks }
        return program, true
    }

    fun := parse_function(p) or_return
    program = new(AST.Block)
    program^ = fun^

    return program, true
}

parse_function :: proc(p: ^Parser) -> (function: ^AST.Function, ok: bool) {
    if eat_tk(p, .CHAR, "-") {
        constant := parse_number(p) or_return
        function = new(AST.Function)
        function^ = AST.Unary_Fun { .MINUS, constant }
        return function, true
    }

    if eat_tk(p, .CHAR, "*") {
        constant := parse_number(p) or_return
        function = new(AST.Function)
        function^ = AST.Unary_Fun { .TIMES, constant }
        return function, true
    }

    if eat_tk(p, .CHAR, "(") {
        program := parse_program(p) or_return
        eat_tk(p, .CHAR, ")") or_return
        function = new(AST.Function)
        function^ = AST.Paren { program }
        return function, true
    }

    if load, kk := eat_tk_and_return(p, .IDENT); kk {
        sb: strings.Builder
        strings.builder_init(&sb)
        strings.write_string(&sb, load)
        for eat_tk(p, .CHAR, "/") {
            strings.write_rune(&sb, '/')
            load = eat_tk_and_return(p, .IDENT) or_return
            strings.write_string(&sb, load)
        }
        function = new(AST.Function)
        function^ = AST.Call_Fun { strings.to_string(sb) }
        return function, true
    }
    return
}

parse_number :: proc(p: ^Parser) -> (number: ^AST.Constant, ok: bool) {
    integer_number_str, int_ok := eat_tk_and_return(p, .INT)
    if int_ok {
        integer_number := strconv.parse_int(integer_number_str, 10) or_return
        number = new(AST.Constant)
        number^ = integer_number
    }

    float_number_str, float_ok := eat_tk_and_return(p, .FLOAT)
    if float_ok {
        float_number := strconv.parse_f32(float_number_str) or_return
        number = new(AST.Constant)
        number^ = float_number
    }
    return number, true
}
