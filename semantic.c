#include "semantic.h"
#include <assert.h>

Type returnType = typNone;

static Type verifyExpr(Ast* ast){
    switch(*ast){
        case astExprInt:
            return typInt32;
        case astExprDouble:
            return typFloat32;
        default:
            //TODO support more expr ast types
            assert(0 && "Invalid AST for expr");
    }
}

static char verifyStmt(Ast* ast){
    switch(*ast){
        case astStmtReturn:
            //TODO auto-casts
            return returnType == verifyExpr(((StmtReturn*)ast)->expr);
        default:
            //TODO support more ast types
            assert(0 && "Invalid AST for stmt");
    }
}
char verifyTopLevel(Ast* ast){
    switch(*ast){
        case astFunction: {
            Function *func = (Function*)ast;
            //TODO check params as well. Right now pretend they are empty
            if (func->stmt == NULL){
                //Declaration only, so no need to verify contents.
                return 1;
            }
            returnType = func->type;
            char correct = verifyStmt(func->stmt);
            returnType = typNone;
            return correct;
        }
        default:
            //TODO support more top level ast types
            assert(0 && "Invalid AST for top level");
    }
}