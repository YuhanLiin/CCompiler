#pragma once
#include "ast.h"

#define SyntaxError 0

void initParser();

Ast* parseExpr();

char isError();