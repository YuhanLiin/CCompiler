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
            ExprStr* str = (ExprStr*)ast;
            free(str->str);
            break;
        }   
        case astExprIdent: {
            ExprIdent* ident = (ExprIdent*)ast;
            free(ident->name);
            break;
        }
        case astExprCall: {
            ExprCall* call = (ExprCall*)ast;
            free(call->name);
            disposeArr(vptr)(&call->args);
            break;
        }
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)binop;
            disposeAst(binop->left);
            disposeAst(binop->right);
            break;
        }
        case astStmtReturn: {
            disposeAst(((StmtReturn*)ast)->expr);
            break;
        }
        case astFunction: {
            Function* func = (Function*)ast;
            disposeAst(func->name);
            if (func->stmt != NULL){
                disposeAst(func->stmt);
            }
            disposeArr(Type)(&func->paramTypes);
            disposeArr(vptr)(&func->paramNames);
        }
        //TODO more delete operations
        default:
            assert(0 && "Unhandled ast label"); //Nothing should fall thru the cracks
    }
    free(ast);
}