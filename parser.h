#pragma once
#include "ast.h"

void initParser();
char checkParse();
Ast* parseStmt();
Ast* parseTopLevel();

char isError();