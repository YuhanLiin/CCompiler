#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "ast.h"

// Assume all declarations have been trimmed
void emitLine(const char* format, ...);

// For expression args
char exprArgStr[30];

void cmplExpr(Ast* ast){
    switch(*ast){
        case astExprInt:
            exprArgStr[0] = '$';
            sprintf(&exprArgStr[1], "%d", ((ExprInt*)ast)->num);
        default:
            assert(0 && "Invalid AST for expr");
    }
}

void cmplStmt(Ast* ast){
    switch(*ast){
        case astStmtReturn:
            cmplExpr((Ast*)((StmtReturn*)ast)->expr);
            emitLine("movq %s, %%rax", exprArgStr);
            break;
        default:
            assert(0 && "Invalid AST for stmt");
    }
}

void cmplTopLevel(Ast* ast){
    switch(*ast){
        case astFunction:{
            Function* func = (Function*)ast;
            emitLine("%s:", func->name);
            emitLine("\tpushq %%rbp");
            emitLine("\tmovq %%rsp, %%rbp");
            if (!strcmp(func->name, "main")){
                emitLine("\tsubq $32 %%rsp");
                emitLine("\tcall __main");
            }
            cmplStmt((Ast*)func->stmt);
            emitLine("leave");
            emitLine("ret");
            break;
        }
        default:
            assert(0 && "Invalid AST for top level");
    }
}