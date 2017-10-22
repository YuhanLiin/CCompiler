#include "semantic.h"
#include "symbols.h"
#include <assert.h>
#include <string.h>

static Type returnType = typNone;
static char correct = 1;

void initSemantics(){
    correct = 1;
}
char checkSemantics(){
    return correct;
}

static char semanticError(){
    //TODO actual error messages
    correct = 0;
    return 0;
}

static char isVarDefined(const StmtVar* var){
    var = findVarCurScope(var->name);
    return var != NULL && var->label == astStmtDef;
}

static char isFuncDefined(const Function* func){
    func = findFunc(func->name);
    return func != NULL && func->stmt != NULL;
}

static void verifyArgs(const ExprCall* call, const Function* func){
    const Array(vptr)* args = &call->args;
    const Array(vptr)* params = &func->params;
    if (args->size == params->size){
        for (size_t i=0; i<args->size; i++){
            if (((ExprBase*)args->elem[i])->type != ((StmtVar*)params->elem[i])->type){
                semanticError();
            }
        }
        return;
    }
    semanticError();
}

//Fills in the type attributes. Returns whether the type of the expression was possible to discern
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
        case astExprIdent: {
            const StmtVar* value = findVar(((ExprIdent*)base)->name);
            if (value){
                base->type = value->type;
                return 1;
            }
            return semanticError();
        }
        case astExprCall: {
            ExprCall* call = (ExprCall*)base;
            const Function* func = findFunc(call->name);
            if (func){
                base->type = func->type;
                verifyArgs(call, func);
                return 1;
            }
            return semanticError();
        }
        default:
            //TODO support more expr ast types
            assert(0 && "Invalid AST for expr");
    }
}

static void verifyStmt(Ast* ast){
    switch(*ast){
        case astStmtReturn:{
            //TODO auto-casts
            StmtReturn* ret = (StmtReturn*)ast;
            if (verifyExpr(ret->expr)){
                assert(ret->expr->type != typNone);
                if (ret->expr->type != returnType){
                    semanticError();
                }
            }
            return;
        }
        case astStmtEmpty:
            return;
        case astStmtExpr:
            verifyExpr(((StmtExpr*)ast)->expr);
        case astStmtBlock:
            return;
        default:
            //TODO support more ast types
            assert(0 && "Invalid AST for stmt");
    }
}

static void verifyAndSetParams(Array(vptr)* params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        if (param->name == NULL || isVarDefined(param)){
            semanticError();
        }
        else{
            insertVar(param->name, param);
        }
    }
}

void verifyTopLevel(Ast* ast){
    switch(*ast){
        case astFunction: {
            Function *func = (Function*)ast;
            if (isFuncDefined(func)){
                semanticError();
            }
            insertFunc(func->name, func);
            if (func->stmt == NULL){
                //Declaration only, so no more content to verify
                return;
            }
            toNewScope();
            verifyAndSetParams(&func->params);
            returnType = func->type;
            verifyStmt(func->stmt);
            returnType = typNone;
            toPrevScope();
            return;
        }
        default:
            //TODO support more top level ast types
            assert(0 && "Invalid AST for top level");
    }
}