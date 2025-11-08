package main

import "core:fmt"
import "core:os"
import "src/AST"
import "src/Lexer"
import "src/Parser"
import "src/Typechecker"
import "src/IR"

main :: proc() {
    /* filepath := "examples/images/rgb_to_grayscale.pipe" */
    filepath := "examples/images/invert_colors.pipe"
    file, err := os.read_entire_file(filepath)

    fmt.println("Source:")
    fmt.println(string(file))

    lexer := Lexer.lexer_make(file)
    tokens := make([dynamic]Lexer.Token)
    Lexer.lexer_tokenize(&lexer, &tokens)

    parser := Parser.Parser { tokens[:], 0 }
    program, ok := Parser.parse_program(&parser)
    AST.print_program(program)

    fmt.println("#########")
    fmt.println("#########")

    ir, kk := IR.program_to_IR(program)
    IR.print_ir_expr(ir)

    IR.print_ir(ir)

    fmt.println()

}
