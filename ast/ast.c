#include "ast/ast.h"
#include "array.h"
#include "lexer/lexer.h"
#include "assert.h"
#include "utils.h"
#include "ast/type.h"
#include <stdlib.h>
#include <stdint.h>

#define Ast(label, lineNumber, linePos) (Ast){label, lineNumber, linePos}
#define ExprBase(ast) (ExprBase){ast, typNone}

ExprDouble* newExprDouble(size_t lineNumber, size_t linePos, double num){
    New(ExprDouble, expr, 1)
    expr->base = ExprBase(Ast(astExprDouble, lineNumber, linePos));
    expr->num = num;
    return expr;
}
ExprFloat* newExprFloat(size_t lineNumber, size_t linePos, float num){
    New(ExprFloat, expr, 1)
    expr->base = ExprBase(Ast(astExprFloat, lineNumber, linePos));
    expr->num = num;
    return expr;
}

ExprLong* newExprLong(size_t lineNumber, size_t linePos, uint64_t num){
    New(ExprLong, expr, 1)
    expr->base = ExprBase(Ast(astExprLong, lineNumber, linePos));
    expr->num = num;
    return expr;
}

ExprInt* newExprInt(size_t lineNumber, size_t linePos, uint32_t num){
    New(ExprInt, expr, 1)
    expr->base = ExprBase(Ast(astExprInt, lineNumber, linePos));
    expr->num = num;
    return expr;
}

ExprStr* newExprStr(size_t lineNumber, size_t linePos, char_t* str){
    New(ExprStr, expr, 1)
    expr->base = ExprBase(Ast(astExprStr, lineNumber, linePos));
    expr->str = str;
    return expr;
}

ExprIdent* newExprIdent(size_t lineNumber, size_t linePos, char_t* name){
    New(ExprIdent, expr, 1)
    expr->base = ExprBase(Ast(astExprIdent, lineNumber, linePos));
    expr->name = name;
    return expr;
}

ExprUnop* newExprUnop(size_t lineNumber, size_t linePos, Token op, ExprBase* operand, char leftside){
    New(ExprUnop, expr, 1)
    expr->base = ExprBase(Ast(astExprUnop, lineNumber, linePos));
    expr->op = op;
    expr->operand = operand;
    expr->leftside = leftside;
    return expr;
}

ExprBinop* newExprBinop(size_t lineNumber, size_t linePos, Token op, ExprBase* left, ExprBase* right){
    New(ExprBinop, expr, 1)
    *expr = (ExprBinop){ExprBase(Ast(astExprBinop, lineNumber, linePos)), op, left, right};
    return expr;
}

ExprCall* newExprCall(size_t lineNumber, size_t linePos, char_t* name){
    New(ExprCall, expr, 1)
    expr->base = ExprBase(Ast(astExprCall, lineNumber, linePos));
    expr->name = name;
    arrInit(vptr)(&expr->args, 0, NULL, &disposeAst);
    return expr;
}

Ast* newStmtEmpty(size_t lineNumber, size_t linePos){
    New(Ast, stmt, 1)
    *stmt = Ast(astStmtEmpty, lineNumber, linePos);
    return stmt;
}
Ast* newStmtBreak(size_t label, size_t lineNumber){
    New(Ast, stmt, 1)
    *stmt = Ast(astStmtBreak, lineNumber, linePos);
    return stmt;
}
Ast* newStmtContinue(size_t label, size_t lineNumber){
    New(Ast, stmt, 1)
    *stmt = Ast(astStmtContinue, lineNumber, linePos);
    return stmt;
}

StmtReturn* newStmtReturn(size_t lineNumber, size_t linePos){
    New(StmtReturn, stmt, 1)
    *stmt = (StmtReturn){Ast(astStmtReturn, lineNumber, linePos), NULL};
    return stmt;
}

StmtExpr* newStmtExpr(size_t lineNumber, size_t linePos, ExprBase* expr){
    New(StmtExpr, stmt, 1)
    *stmt = (StmtExpr){Ast(astStmtExpr, lineNumber, linePos), expr};
    return stmt;
}

StmtBlock* newStmtBlock(size_t label, size_t lineNumber){
    New(StmtBlock, blk, 1)
    blk->ast = Ast(astStmtBlock, lineNumber, linePos);
    arrInit(vptr)(&blk->stmts, 0, NULL, &disposeAst);
    return blk;
}

StmtVar* newStmtVarDef(size_t lineNumber, size_t linePos, Type type, char_t* name){
    New(StmtVar, stmt, 1)
    *stmt = (StmtVar){Ast(astStmtDef, lineNumber, linePos), type, name, NULL};
    return stmt;
}

StmtVar* newStmtVarDecl(size_t lineNumber, size_t linePos, Type type, char_t* name){
    New(StmtVar, stmt, 1)
    *stmt = (StmtVar){Ast(astStmtDecl, lineNumber, linePos), type, name, NULL};
    return stmt;
}

StmtWhileLoop* newStmtWhile(size_t lineNumber, size_t linePos, ExprBase* condition, Ast* stmt){
    New(StmtWhileLoop, loop, 1)
    loop->ast = Ast(astStmtWhile, lineNumber, linePos);
    loop->condition = condition;
    loop->stmt = stmt;
    return loop;
}
StmtWhileLoop* newStmtDoWhile(size_t lineNumber, size_t linePos, ExprBase* condition, Ast* stmt){
    New(StmtWhileLoop, loop, 1)
    loop->ast = Ast(astStmtDoWhile, lineNumber, linePos);
    loop->condition = condition;
    loop->stmt = stmt;
    return loop;
}

StmtIf* newStmtIf(size_t lineNumber, size_t linePos, ExprBase* condition, Ast* ifStmt, Ast* elseStmt){
    New(StmtIf, ifelse, 1)
    ifelse->ast = Ast(astStmtIf, lineNumber, linePos);
    ifelse->condition = condition;
    ifelse->ifStmt = ifStmt;
    ifelse->elseStmt = elseStmt;
    return ifelse;
}

Function* newFunction(size_t lineNumber, size_t linePos, Type type, char_t* name){
    New(Function, func, 1)
    func->ast = Ast(astFunction, lineNumber, linePos);
    func->type = type;
    func->name = name;
    func->stmt = NULL;
    arrInit(vptr)(&func->params, 0, NULL, &disposeAst);
    return func;
}

TopLevel* newTopLevel(){
    New(TopLevel, toplevel, 1);
    toplevel->ast = Ast(astTopLevel, lineNumber, linePos);
    arrInit(vptr)(&toplevel->globals, 0, NULL, &disposeAst);
    return toplevel;
}

char isVarDecl(const StmtVar* var){
    return var->ast.label == astStmtDecl;
}

char isFuncDecl(const Function* func){
    return func->stmt == NULL;
}

char hasRetExpr(const StmtReturn* ret){
    return ret->expr != NULL;
}

//Delete ast node based on ast label
void disposeAst(void* node){
    if (node == NULL) return;
    Ast* ast = node;
    switch(ast->label){
        case astExprDouble:
        case astExprInt:
        case astExprLong:
        case astExprFloat:
        case astStmtEmpty:
        case astStmtBreak:
        case astStmtContinue:
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
        case astExprUnop: {
            disposeAst(((ExprUnop*)ast)->operand);
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
        case astStmtExpr:
            disposeAst(((StmtExpr*)ast)->expr);
            break;
        case astStmtBlock:
            arrDispose(vptr)(&((StmtBlock*)ast)->stmts);
            break;
        case astStmtDecl:
        case astStmtDef: {
            StmtVar* var = (StmtVar*)ast;
            disposeAst(var->rhs);
            free(var->name);
            break;
        }
        case astStmtDoWhile:
        case astStmtWhile: {
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            disposeAst(loop->condition);
            disposeAst(loop->stmt);
            break;
        }
        case astStmtIf: {
            StmtIf* ifelse = (StmtIf*)ast;
            disposeAst(ifelse->condition);
            disposeAst(ifelse->ifStmt);
            disposeAst(ifelse->elseStmt);
            break;
        }
        case astFunction: {
            Function* func = (Function*)ast;
            free(func->name);
            disposeAst(func->stmt);
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