#pragma once
#include "ast/type.h"
#include "ast/ast.h"

void initSemantics();
char checkSemantics();
void verifyTopLevel(TopLevel* ast);