#pragma once
#include "utils.h"

typedef struct {
    char_t* name;
    size_t scopeId;
} Symbol;

#define GLOBAL_SCOPE 0
extern size_t curScope;

size_t hashSymbol(Symbol sym);

char eqSymbol(Symbol a, Symbol b);

size_t toNewScope();

size_t toPrevScope();

void resetScopes();

void initScopes();

void disposeScopes();

size_t prevScope(size_t scopeId);