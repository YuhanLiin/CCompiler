#include "symbols.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* name;
    size_t scopeId;
} Symbol;
#define Symbol(name, id) (Symbol){name, id}

typedef Ast* Astptr;
#define KEY Symbol
#define VAL Astptr
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

#define GLOBAL_SCOPE 0
size_t curScope = GLOBAL_SCOPE;
Array(size_t) parentScopes; //Mapping between scope IDs and their parents
Map(Symbol, Astptr) symbolTable;

static size_t hashSymbol(Symbol sym){
    size_t hash = 5381;
    for (size_t i=0; sym.name[i]; i++){
        hash = hash * 33 + sym.name[i];
    }
    return hash * 33 + sym.scopeId;
}

static char eqSymbol(Symbol a, Symbol b){
    return !strcmp(a.name, b.name) && a.scopeId == b.scopeId;
}

static void cleanSymbol(Symbol sym){
    free(sym.name);
}

void initSymbolTable(){
    mapInit(Symbol, Astptr)(&symbolTable, 4, &hashSymbol, &eqSymbol, &cleanSymbol, NULL);
    arrInit(size_t)(&parentScopes, GLOBAL_SCOPE + 1, NULL, NULL);
    curScope = GLOBAL_SCOPE;
}

void disposeSymbolTable(){
    mapDispose(Symbol, Astptr)(&symbolTable);
    arrDispose(size_t)(&parentScopes);
}

size_t newScope(){
    //Largest scope ID is n-1, where n is size of parentScopes.
    //New largest scope ID will be n with curScope as parent. Size of parentScope increases as well
    if (!arrPush(size_t)(&parentScopes, curScope)){
        exit(1);
    }
    return curScope = parentScopes.size - 1;
}

size_t prevScope(){
    return curScope = parentScopes.elem[curScope];
}

void insertSymbol(char* name, size_t scopeId, Ast* ast){
    if (!mapInsert(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId), ast)){
        exit(1);
    }
}

Ast* findSymbol(char* name, size_t scopeId, char fullLookup){
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));
    if (fullLookup){
        while (astptr == NULL && scopeId != GLOBAL_SCOPE){
            scopeId = parentScopes.elem[scopeId];
            astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));
        }
    }
    return *astptr;
}
