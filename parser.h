#pragma once
#include "ast.h"

void initParser();

ExprBase* parseExpr();
Ast* parseStmt();
Ast* parseTopLevel();

char isError();