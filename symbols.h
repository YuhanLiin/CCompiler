#pragma once
#include "ast.h"

void initSymbolTable();

void disposeSymbolTable();

size_t newScope();

size_t prevScope();

void insertSymbol(char* name, size_t scopeId, Ast* ast);

Ast* findSymbol(char* name, size_t scopeId, char fullLookup);
