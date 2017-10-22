#pragma once
#include "ast.h"

void initSymbolTable();

void disposeSymbolTable();

size_t toNewScope();

size_t toPrevScope();

void insertVar(char* name, StmtVar* expr);

void insertFunc(char* name, Function* func);

const StmtVar* findVarCurScope(char_t* name);

const StmtVar* findVar(char* name);

const Function* findFunc(char* name);
