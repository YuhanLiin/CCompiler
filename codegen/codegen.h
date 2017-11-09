#pragma once
#include "utils.h"
#include "ast/ast.h"

void cmplTopLevel(TopLevel* ast);

// IO command. Not supplied by codegen/codegen.c
void emitAsm(const char* format, ...);