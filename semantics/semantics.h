#pragma once
#include "ast/type.h"
#include "ast/ast.h"

void initSemantics();
char checkSemantics();

ExprInt* verifyExprInt(ExprInt* expint);
ExprInt* verifyExprUnsignedInt(ExprInt* expint);
ExprLong* verifyExprLong(ExprLong* expint);
ExprLong* verifyExprUnsignedLong(ExprLong* expint);

ExprDouble* verifyExprDouble(ExprDouble* expdb);
ExprFloat* verifyExprFloat(ExprFloat* expdb);

ExprUnop* verifyExprUnop(ExprUnop* unop);
ExprBinop* verifyExprBinop(ExprBinop* binop);
ExprIdent* verifyExprIdent(ExprIdent* ident);
ExprCall* verifyExprCall(ExprCall* call);

void preverifyBlockStmt();
void postVerifyBlockStmt();
StmtBlock* verifyBlockStmt(StmtBlock* blk);
StmtReturn* verifyStmtReturn(StmtReturn* ret);
StmtVar* verifyStmtVar(StmtVar* var);

void verifyFunctionSignature(Function* func, char isDecl);
void verifyFunctionBody();
