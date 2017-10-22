#pragma once
#include "type.h"
#include "ast.h"

void initSemantics();
char checkSemantics();
void verifyTopLevel(TopLevel* ast);