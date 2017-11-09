#pragma once
#include "ast/ast.h"

void initParser();
char checkParse();
Ast* parseStmt();
TopLevel* parseTopLevel();

char isError();