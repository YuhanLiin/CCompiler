#include "symbols.h"
#include "utils.h"
#include "array.h"
#include <stdlib.h>

size_t curScope = GLOBAL_SCOPE;
static Array(size_t) parentScopes; //Mapping between scope IDs and their parents

void initScopes(){
    arrInit(size_t)(&parentScopes, GLOBAL_SCOPE + 1, NULL, NULL);
    curScope = GLOBAL_SCOPE;
}

void disposeScopes(){
    arrDispose(size_t)(&parentScopes);
}

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

size_t toNewScope(){
    //Largest scope ID is n-1, where n is size of parentScopes.
    //New largest scope ID will be n with curScope as parent. Size of parentScope increases as well
    if (!arrPush(size_t)(&parentScopes, curScope)){
        exit(1);
    }
    return curScope = parentScopes.size - 1;
}

size_t prevScope(size_t scopeId){
    return parentScopes.elem[curScope];
}

size_t toPrevScope(){
    return curScope = prevScope(curScope);
}

