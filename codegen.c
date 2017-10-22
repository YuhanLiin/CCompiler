#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "ast.h"
#include "codegen.h"

// Assume all declarations have been trimmed out

// For expression args
static char exprArgStr[30];

void cmplExpr(Ast* ast){
    switch(*ast){
        case astExprInt:
            exprArgStr[0] = '$';
            sprintf(&exprArgStr[1], "%d", ((ExprInt*)ast)->num);
            break;
        default:
            assert(0 && "Invalid AST for expr");
    }
}

void cmplStmt(Ast* ast){
    switch(*ast){
        case astStmtReturn:
            cmplExpr((Ast*)((StmtReturn*)ast)->expr);
            emitLine("\tmovq %s, %%rax", exprArgStr);
            break;
        default:
            assert(0 && "Invalid AST for stmt");
    }
}

static void cmplGlobal(Ast* ast){
    // Hardcode text section for now
    emitLine(".text");
    emitLine("\t.globl main");
    switch(*ast){
        case astFunction:{
            Function* func = (Function*)ast;
            emitLine("%s:", func->name);
            emitLine("\tpushq %%rbp");
            emitLine("\tmovq %%rsp, %%rbp");
            if (!strcmp(func->name, "main")){
                emitLine("\tsubq $32, %%rsp");
                emitLine("\tcall __main");
            }
            cmplStmt((Ast*)func->stmt);
            emitLine("\tleave");
            emitLine("\tret");
            break;
        }
        default:
            assert(0 && "Invalid AST for top level");
    }
}

void cmplTopLevel(TopLevel* top){
    for (size_t i=0; i<top->globals.size; i++){
        cmplGlobal(top->globals.elem[i]);
    }
}