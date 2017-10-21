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


void initSymbolTable(){
    mapInit(Symbol, Astptr)(&symbolTable, 4, &hashSymbol, &eqSymbol, NULL, NULL);
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

static void insertSymbol(char* name, Ast* ast){
    if (!mapInsert(Symbol, Astptr)(&symbolTable, Symbol(name, curScope), ast)){
        exit(1);
    }
}

char insertVar(char* name, ExprBase* expr){
    //Variable can be defined if it hasnt been defined before
    if (!mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, curScope))){
        insertSymbol(name, &expr->label);
        return 1;
    }
    return 0;
}

char insertFunc(char* name, Function* func){
    Function* prev = findFunc(name);
    //Function can only be defined if function of same name hasn't been defined before
    if (prev == NULL || prev->stmt == NULL){
        insertSymbol(name, &func->label);
        return 1;
    }
    return 0;
}

//Search for a variable from the current to the global scope
const ExprBase* findVar(char* name){
    size_t scopeId = curScope;
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));
    while (astptr == NULL && scopeId != GLOBAL_SCOPE){
        scopeId = parentScopes.elem[scopeId];
        astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));
    }
    if (astptr && **astptr != astFunction){
        return (ExprBase*)(*astptr);
    }
    return NULL;
}

//Search for a function in the global scope
const Function* findFunc(char* name){
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, GLOBAL_SCOPE));
    if (astptr && **astptr == astFunction){
        return (Function*)(*astptr);
    }
    return NULL;
}
