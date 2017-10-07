#include "semantic.h"
#include <assert.h>
#include <string.h>

Type returnType = typNone;

void semanticError(){
    //TODO actual error messages
}

// Fills in the type attributes and verifies they are compatible
static char verifyExpr(ExprBase* base){
    switch(base->label){
        case astExprInt:
            base->type = typInt32;
            return 1;
        case astExprDouble:
            base->type = typFloat32;
            return 1;
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)base;
            if (verifyExpr(binop->left) && verifyExpr(binop->right)){
                assert(binop->left->type != typNone);
                assert(binop->right->type != typNone);
                if (binop->left->type == binop->right->type){
                    base->type = binop->left->type;
                    return 1;
                }
                semanticError();
            }
            return 0;
        }
        default:
            //TODO support more expr ast types
            assert(0 && "Invalid AST for expr");
    }
}

static char verifyStmt(Ast* ast){
    switch(*ast){
        case astStmtReturn:{
            //TODO auto-casts
            StmtReturn* ret = (StmtReturn*)ast;
            if (verifyExpr(ret->expr)){
                assert(ret->expr->type != typNone);
                if (ret->expr->type == returnType){
                    return 1;
                }
                semanticError();
            }
            return 0;
        }
        case astStmtEmpty:
            return 1;
        case astStmtExpr:
            if (verifyExpr(((StmtExpr*)ast)->expr)){
                return 1;
            }
            return 0;
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
            char valid = verifyStmt(func->stmt);
            returnType = typNone;
            return valid && !strcmp(func->name, "main");
        }
        default:
            //TODO support more top level ast types
            assert(0 && "Invalid AST for top level");
    }
}