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
        case astExprNum:
            free((ExprNum*)ast);
            return;
        case astExprStr: {
            ExprStr* str = (ExprStr*)ast;
            free(str->str);
            free(str);
            return;
        }   
        case astExprIdent: {
            ExprIdent* ident = (ExprIdent*)ast;
            free(ident->name);
            free(ident);
            return;
        }
        case astExprCall: {
            ExprCall* call = (ExprCall*)ast;
            free(call->name);
            disposeArr(vptr)(&call->args);
            free(call);
            return;
        }
        //TODO more delete operations
        default:
            assert(0 && "Unhandled ast label"); //Nothing should fall thru the cracks
    }
}