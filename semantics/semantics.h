#pragma once
#include "ast/type.h"
#include "ast/ast.h"

void initSemantics();
char checkSemantics();

//void verifyTopLevel(TopLevel* ast);
ExprInt* verifyExprInt(ExprInt* expint);
ExprDouble* verifyExprDouble(ExprDouble* expdb);
ExprBinop* verifyExprBinop(ExprBinop* binop);
ExprIdent* verifyExprIdent(ExprIdent* ident);
ExprCall* verifyExprCall(ExprCall* call);

void preverifyBlockStmt();
StmtBlock* verifyBlockStmt(StmtBlock* blk);
StmtReturn* verifyStmtReturn(StmtReturn* ret);

void preverifyFunction(Function* func, char isDecl);
void verifyFunctionDefinition();
