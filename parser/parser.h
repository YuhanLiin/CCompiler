#pragma once
#include "ast/ast.h"

void initParser();
void disposeParser();

char checkSyntax();
Ast* parseStmt();
TopLevel* parseTopLevel();
