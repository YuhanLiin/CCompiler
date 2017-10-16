#include "ast.h"
#include "array.h"
#include "lexer.h"
#include "assert.h"
#include "utils.h"
#include <stdlib.h>

//Delete ast node based on ast label
void disposeAst(void* node){
    Ast* ast = node;
    switch(*ast){
        case astExprDouble:
            break;
        case astExprInt:
            break;
        case astExprStr: {
            free(((ExprStr*)ast)->str);
            break;
        }   
        case astExprIdent: {
            free(((ExprIdent*)ast)->name);
            break;
        }
        case astExprCall: {
            ExprCall* call = (ExprCall*)ast;
            free(call->name);
            arrDispose(vptr)(&call->args);
            break;
        }
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)ast;
            disposeAst(binop->left);
            disposeAst(binop->right);
            break;
        }
        case astStmtReturn: {
            disposeAst(((StmtReturn*)ast)->expr);
            break;
        }
        case astStmtEmpty:
            break;
        case astStmtExpr:
            disposeAst(((StmtExpr*)ast)->expr);
            break;
        case astStmtBlock:
            arrDispose(vptr)(&((StmtBlock*)ast)->stmts);
            break;
        case astStmtDecl:
            free(((StmtDecl*)ast)->name);
            break;
        case astFunction: {
            Function* func = (Function*)ast;
            free(func->name);
            if (func->stmt != NULL){
                disposeAst(func->stmt);
            }
            arrDispose(vptr)(&func->params);
            break;
        }
        //TODO more delete operations
        default:
            assert(0 && "Unhandled ast label");
    }
    free(ast);
}