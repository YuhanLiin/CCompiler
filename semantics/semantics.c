#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
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
    const Ast* sym = findSymbolCurScope(var->name);
    // If variable name exists and is not an externed variable, the new var can't be defined
    return sym != NULL && *sym != astStmtDecl;
}

static char isFuncDefined(const Function* func){
    const Ast* sym = findSymbolCurScope(func->name);
    //If name doesn't exist, then we can define new func
    if (sym == NULL){
        return 0;
    }
    //If the symbol is a variable or func definition, we cannot define new func
    const Function* prev = (const Function*)sym;
    if (*sym != astFunction || !isFuncDecl(prev)){
        return 1;
    }
    // Even if previous occurrence was a declaration, still need to check whether the signatures are equal
    if (func->type == prev->type && func->params.size == prev->params.size){
        for (size_t i=0; i<func->params.size; i++){
            if (((StmtVar*)func->params.elem[i])->type != ((StmtVar*)func->params.elem[i])->type){
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static char verifyExpr(ExprBase* expr);

static void verifyArgs(const ExprCall* call, const Function* func){
    const Array(vptr)* args = &call->args;
    const Array(vptr)* params = &func->params;
    if (args->size == params->size){
        for (size_t i=0; i<args->size; i++){
            ExprBase* arg = (ExprBase*)args->elem[i];
            StmtVar* param = (StmtVar*)params->elem[i];
            if (verifyExpr(arg)){
                if (arg->type != param->type){
                    semanticError();
                }  
            }
        }
        return;
    }
    semanticError();
}

//Fills in the type attributes. Returns whether the type of the expression was possible to discern
static char verifyExpr(ExprBase* expr){
    switch(expr->label){
        case astExprInt:
            expr->type = typInt32;
            return 1;
        case astExprDouble:
            expr->type = typFloat32;
            return 1;
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)expr;
            if (verifyExpr(binop->left) && verifyExpr(binop->right)){
                assert(binop->left->type != typNone);
                assert(binop->right->type != typNone);
                if (binop->left->type == binop->right->type){
                    expr->type = binop->left->type;
                    return 1;
                }
                semanticError();
            }
            return 0;
        }
        case astExprIdent: {
            const StmtVar* value = findVar(((ExprIdent*)expr)->name);
            if (value){
                expr->type = value->type;
                return 1;
            }
            return semanticError();
        }
        case astExprCall: {
            ExprCall* call = (ExprCall*)expr;
            const Function* func = findFunc(call->name);
            if (func){
                expr->type = func->type;
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

static void verifyStmt(Ast* ast);

static void verifyBlockStmt(StmtBlock* blk){
    for (size_t i=0; i<blk->stmts.size; i++){
        verifyStmt(blk->stmts.elem[i]);
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
            return;
        case astStmtBlock: {
            toNewScope();
            ((StmtBlock*)ast)->scopeId = curScope;
            verifyBlockStmt((StmtBlock*)ast);
            toPrevScope();
            return;
        }
        default:
            //TODO support more ast types
            assert(0 && "Invalid AST for stmt");
    }
}

static void verifyAndSetParams(Array(vptr)* params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        if (param->name == NULL){
            semanticError();
        }
        else if (isVarDefined(param)){
            semanticError();
        }
        else{
            insertVar(param->name, param);
        }
    }
}

static void verifyGlobal(Ast* ast){
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
            func->scopeId = curScope;
            verifyAndSetParams(&func->params);
            returnType = func->type;
            if (*func->stmt == astStmtBlock){
                verifyBlockStmt((StmtBlock*)func->stmt);
                ((StmtBlock*)func->stmt)->scopeId = curScope;
            }
            else {
                verifyStmt(func->stmt);
            }
            returnType = typNone;
            toPrevScope();
            return;
        }
        default:
            //TODO support more top level ast types
            assert(0 && "Invalid AST for top level");
    }
}

void verifyTopLevel(TopLevel* top){
    assert(top != NULL && "Can't perform semantic analysis on bad AST");
    for (size_t i=0; i<top->globals.size; i++){
        verifyGlobal(top->globals.elem[i]);
    }
}