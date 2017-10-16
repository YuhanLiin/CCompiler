#include "symbols.h"
#include <stdlib.h>
#include <string.h>

#define KEY Symbol
#define VAL Type
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

size_t curScope = GLOBAL_SCOPE;
size_t maxScope = GLOBAL_SCOPE;
Array(size_t) parentScopes; //Mapping between scope IDs and their parents
Map(Symbol, Type) symbolTable;

size_t hashSymbol(Symbol sym){
    size_t hash = 5381;
    for (size_t i=0; sym.name[i]; i++){
        hash = hash * 33 + sym.name[i];
    }
    return hash * 33 + sym.scopeId;
}

char eqSymbol(Symbol a, Symbol b){
    return !strcmp(a.name, b.name) && a.scopeId == b.scopeId;
}

void cleanSymbol(Symbol sym){
    free(sym.name);
}

void initSymbols(){
    mapInit(Symbol, Type)(&symbolTable, 4, &hashSymbol, &eqSymbol, &cleanSymbol, NULL);
    arrInit(size_t)(&parentScopes, GLOBAL_SCOPE + 1, NULL, NULL);
}

size_t newScope(){
    return curScope = ++maxScope;
}

size_t prevScope(){
    return curScope = parentScopes->elem[curScope];
}