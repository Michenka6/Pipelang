package AST

import "core:strings"
import "core:fmt"
RED :: "\033[31m"
GREEN :: "\033[32m"
YELLOW :: "\033[38;5;226m"
BLUE :: "\033[38;5;27m"
RESET :: "\033[0m"


print_prefix :: proc(prefix: string, is_last: bool) -> string {
    if is_last {
        return fmt.tprintf("%s%s", prefix, "└── ")
    } else {
        return fmt.tprintf("%s%s", prefix, "├── ")
    }
}

next_prefix :: proc(prefix: string, is_last: bool) -> string {
    if is_last {
        return fmt.tprintf("%s%s", prefix, "    ")
    } else {
        return fmt.tprintf("%s%s", prefix, "│   ")
    }
}

print_program :: proc(p: ^Program, prefix: string = "", is_last: bool = false) {
    fmt.println(print_prefix(prefix, is_last), "Program")

    for b, i in p.blocks {
        print_block(b, next_prefix(prefix, is_last), i == len(p.blocks)-1)
    }
}

print_block :: proc(block: Block, prefix: string, is_last: bool) {
    switch b in block {
        case Sequence:
            fmt.println(print_prefix(prefix, is_last), "Sequence")
            for blk, i in b.blocks {
                print_block(blk, next_prefix(prefix, is_last), i == len(b.blocks)-1)
            }
        case Branch:
            fmt.printf("%sBranch (folder = %v)\n", print_prefix(prefix, is_last), b.folder)
            for blk, i in b.blocks {
                print_block(blk, next_prefix(prefix, is_last), i == len(b.blocks)-1)
            }
        case Function:
            print_function(b, prefix, is_last)
    }
}

print_function :: proc(fun: Function, prefix: string, is_last: bool) {
    switch f in fun {
        case Call_Fun:
            fmt.printf("%sCall_Fun (name = %v)\n", print_prefix(prefix, is_last), f.name)
        case Unary_Fun:
            fmt.printf("%sUnary_Fun (op = %v)\n", print_prefix(prefix, is_last), f.op)
            print_constant(f.param, next_prefix(prefix, is_last), true)
        case Paren:
            fmt.println(print_prefix(prefix, is_last), "Paren")
            print_program(f.program, next_prefix(prefix, is_last), true)
    }
}

print_constant :: proc(const: ^Constant, prefix: string, is_last: bool) {
    switch c in const {
        case int: fmt.printf("%sConstant int = %v\n", print_prefix(prefix, is_last), c)
        case f32: fmt.printf("%sConstant f32 = %v\n", print_prefix(prefix, is_last), c)
    }
}
