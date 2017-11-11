//Mock out semantics.c so that parsertest can run without dumb error messages
#include "ast/type.h"
#include "ast/ast.h"

void initSemantics(){}
char checkSemantics(){}

ExprInt* verifyExprInt(ExprInt* expint){return expint;}
ExprDouble* verifyExprDouble(ExprDouble* expdb){return expdb;}
ExprBinop* verifyExprBinop(ExprBinop* binop){return binop;}
ExprIdent* verifyExprIdent(ExprIdent* ident){return ident;}
ExprCall* verifyExprCall(ExprCall* call){return call;}

void preverifyBlockStmt(){}
StmtBlock* verifyBlockStmt(StmtBlock* blk){return blk;}
StmtReturn* verifyStmtReturn(StmtReturn* ret){return ret;}

void preverifyFunction(Function* func, char isDecl){}
void verifyFunctionDefinition(){}
