#include "ast.h"
#include "array.h"
#include "lexer.h"
#include "assert.h"
#include "utils.h"
#include <stdlib.h>

ExprDouble* newExprDouble(double num){
    New(ExprDouble, expr, 1)
    expr->base = ExprBase(astExprDouble);
    expr->num = num;
    return expr;
}

ExprInt* newExprInt(long long num){
    New(ExprInt, expr, 1)
    expr->base = ExprBase(astExprInt);
    expr->num = num;
    return expr;
}

ExprStr* newExprStr(char_t* str){
    New(ExprStr, expr, 1)
    expr->base = ExprBase(astExprStr);
    expr->str = str;
    return expr;
}

ExprIdent* newExprIdent(char_t* name){
    New(ExprIdent, expr, 1)
    expr->base = ExprBase(astExprIdent);
    expr->name = name;
    return expr;
}

ExprBinop* newExprBinop(Token op, ExprBase* left, ExprBase* right){
    New(ExprBinop, expr, 1)
    *expr = (ExprBinop){ExprBase(astExprBinop), op, left, right};
    return expr;
}

ExprCall* newExprCall(char_t* name){
    New(ExprCall, expr, 1)
    expr->base = ExprBase(astExprCall);
    expr->name = name;
    arrInit(vptr)(&expr->args, 0, NULL, &disposeAst);
    return expr;
}

StmtEmpty* newStmtEmpty(){
    New(StmtEmpty, stmt, 1)
    stmt->label = astStmtEmpty;
    return stmt;
}

StmtReturn* newStmtReturn(ExprBase* expr){
    New(StmtReturn, stmt, 1)
    *stmt = (StmtReturn){astStmtReturn, expr};
    return stmt;
}

StmtExpr* newStmtExpr(ExprBase* expr){
    New(StmtExpr, stmt, 1)
    *stmt = (StmtExpr){astStmtExpr, expr};
    return stmt;
}

StmtBlock* newStmtBlock(){
    New(StmtBlock, blk, 1)
    blk->label = astStmtBlock;
    arrInit(vptr)(&blk->stmts, 0, NULL, &disposeAst);
    return blk;
}

StmtVar* newStmtVarDef(Type type, char_t* name){
    New(StmtVar, stmt, 1)
    *stmt = (StmtVar){astStmtDef, type, name};
    return stmt;
}

StmtVar* newStmtVarDecl(Type type, char_t* name){
    New(StmtVar, stmt, 1)
    *stmt = (StmtVar){astStmtDecl, type, name};
    return stmt;
}

Function* newFunction(Type type, char_t* name){
    New(Function, func, 1)
    *func = (Function){astFunction, type, name, NULL};
    arrInit(vptr)(&func->params, 0, NULL, &disposeAst);
    return func;
}

TopLevel* newTopLevel(){
    New(TopLevel, toplevel, 1);
    toplevel->label = astTopLevel;
    arrInit(vptr)(&toplevel->globals, 0, NULL, &disposeAst);
    return toplevel;
}

char isVarDecl(const StmtVar* var){
    return var->label == astStmtDecl;
}

char isFuncDecl(const Function* func){
    return func->stmt == NULL;
}

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
        case astStmtDef:
            free(((StmtVar*)ast)->name);
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
        case astTopLevel:
            arrDispose(vptr)(&((TopLevel*)ast)->globals);
            break;
        //TODO more delete operations
        default:
            assert(0 && "Unhandled ast label");
    }
    free(ast);
}