#pragma once
#include "ast/ast.h"

void initParser();

char checkSyntax();
Ast* parseStmtOrDef();
TopLevel* parseTopLevel();
