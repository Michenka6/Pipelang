package IR

import "../AST"

program_to_IR :: proc(program: ^AST.Program) -> (e: ^Expr, ok: bool) {
    if program == nil { return }
    current := block_to_IR(&program.blocks[0]) or_return
    for i in 1..<len(program.blocks) {
        block_ir := block_to_IR(&program.blocks[i]) or_return
        e1 := new(Expr)
        e1^ = Concat { current, block_ir }

        current = e1
    }
    return current, true
}

block_to_IR :: proc(block: ^AST.Block) -> (e: ^Expr, ok: bool) {
    if block == nil { return }
    switch b in block {
    case AST.Sequence:
        current := block_to_IR(&b.blocks[0]) or_return
        for i in 1..<len(b.blocks) {
            block_ir := block_to_IR(&b.blocks[i]) or_return
            e1 := new(Expr)
            e1^ = Concat { current, block_ir }

            current = e1
        }
        return current, true
    case AST.Branch:
        current := block_to_IR(&b.blocks[0]) or_return
        for i in 1..<len(b.blocks) {
            block_ir := block_to_IR(&b.blocks[i]) or_return
            e1 := new(Expr)
            e1^ = Fork { current, block_ir }

            current = e1
        }
        e = new(Expr)
        e^ = Unfold { current, b.folder }
        return e, true
    case AST.Function:
        return function_to_IR(transmute(^AST.Function)block)
    }
    return
}

function_to_IR :: proc(function: ^AST.Function) -> (e: ^Expr, ok: bool) {
    switch fun in function {
    case AST.Call_Fun:
        e = new(Expr)
        e^ = Fun { fun.name }
        return e, true
    case AST.Unary_Fun:
        e = new(Expr)
        e^ = Unary_Fun { fun.op, fun.param }
        return e, true
    case AST.Paren:
        return program_to_IR(fun.program)
    }
    return 
}
