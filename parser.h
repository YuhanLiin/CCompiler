#pragma once
#include "ast.h"

void initParser();

Ast* parseExpr();
Ast* parseStmt();
Ast* parseTopLevel();

char isError();