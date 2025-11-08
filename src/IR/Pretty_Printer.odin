package IR

import "core:fmt"
import "core:strings"
import "../AST"

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

print_ir :: proc(expr: ^Expr) {
    switch e in expr {
        case Fun:
            fmt.printf("%s", e.name)

        case Concat:
            print_ir(e.e1)
            fmt.print("; ")
            print_ir(e.e2)

        case Epsillon:
            fmt.print("ε")

        case Fork:
            print_ir(e.e1)
            fmt.print(" + ")
            print_ir(e.e2)

        case Fail:
            fmt.print("∅")

        case Fold:
            fmt.print("⌊")
            print_ir(e.e)
            fmt.printf("⌋_%s", e.g)

        case Unfold:
            fmt.print("⌈")
            print_ir(e.e)
            fmt.printf("⌉_%s", e.g)

        case Unary_Fun:
            fmt.print("(")
            fmt.print(e.param^)
            fmt.print(")")
    }
    return
}

print_ir_expr :: proc(expr: ^Expr, prefix: string = "", is_last: bool = true) {
    switch e in expr {
        case Fun:
            fmt.printf("%sFun (name = %v)\n", print_prefix(prefix, is_last), e.name)

        case Concat:
            fmt.println(print_prefix(prefix, is_last), "Concat")
            print_ir_expr(e.e1, next_prefix(prefix, is_last), false)
            print_ir_expr(e.e2, next_prefix(prefix, is_last), true)

        case Epsillon:
            fmt.println(print_prefix(prefix, is_last), "Epsillon")

        case Fork:
            fmt.println(print_prefix(prefix, is_last), "Fork")
            print_ir_expr(e.e1, next_prefix(prefix, is_last), false)
            print_ir_expr(e.e2, next_prefix(prefix, is_last), true)

        case Fail:
            fmt.println(print_prefix(prefix, is_last), "Fail")

        case Fold:
            fmt.printf("%sFold (g = %v)\n", print_prefix(prefix, is_last), e.g)
            print_ir_expr(e.e, next_prefix(prefix, is_last), true)

        case Unfold:
            fmt.printf("%sUnfold (g = %v)\n", print_prefix(prefix, is_last), e.g)
            print_ir_expr(e.e, next_prefix(prefix, is_last), true)

        case Unary_Fun:
            fmt.printf("%sUnary_Fun (op = %v)\n", print_prefix(prefix, is_last), e.op)
            print_ast_constant(e.param, next_prefix(prefix, is_last), true)
    }
}

print_ast_constant :: proc(const: ^AST.Constant, prefix: string, is_last: bool) {
    switch c in const {
        case int: fmt.printf("%sConstant int = %v\n", print_prefix(prefix, is_last), c)
        case f32: fmt.printf("%sConstant f32 = %v\n", print_prefix(prefix, is_last), c)
    }
}

