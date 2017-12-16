#pragma once
#include "utils.h"
#include "ast/ast.h"

void cmplTopLevel(TopLevel* ast);

void initAsm();
void disposeAsm();

void emitAllAsm();
