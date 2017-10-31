#pragma once
#include "ast.h"

void initSymbolTable();

void disposeSymbolTable();

void insertVar(char* name, StmtVar* expr);

void insertFunc(char* name, Function* func);

const Ast* findSymbolCurScope(char_t* name);

const StmtVar* findVar(char* name);

const Function* findFunc(char* name);