#pragma once
#include "ast.h"

void initSymbolTable();

void disposeSymbolTable();

size_t newScope();

size_t prevScope();

char insertVar(char* name, ExprBase* expr);

char insertFunc(char* name, Function* func);

const ExprBase* findVar(char* name);

const Function* findFunc(char* name);
