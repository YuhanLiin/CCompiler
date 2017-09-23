#pragma once
#include "ast.h"

#define SyntaxError 0

void initParser();

Ast* parseExpr();
Ast* parseStmt();
Ast* parseTopLevel();

char isError();