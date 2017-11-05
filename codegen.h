#pragma once
#include "utils.h"
#include "ast.h"

void cmplTopLevel(TopLevel* ast);

// IO command. Not supplied by codegen.c
void emitAsm(const char* format, ...);