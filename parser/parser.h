#pragma once
#include "ast/ast.h"

void initParser();

char checkSyntax();
Ast* parseStmt();
TopLevel* parseTopLevel();
