#pragma once
#include "ast.h"

void initParser();
char checkParse();
Ast* parseStmt();
TopLevel* parseTopLevel();

char isError();