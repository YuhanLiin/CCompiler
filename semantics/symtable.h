#pragma once
#include "ast/ast.h"
// Main symbol table populated at semantic analysis

void initSymbolTable();

void disposeSymbolTable();

void insertVar(char* name, StmtVar* expr);

void insertFunc(char* name, Function* func);

const Ast* findSymbolCurScope(char_t* name);

const StmtVar* findVar(char* name);

const Function* findFunc(char* name);