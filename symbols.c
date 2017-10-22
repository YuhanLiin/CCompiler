#include "symbols.h"
#include "ast.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char_t* name;
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
static size_t curScope = GLOBAL_SCOPE;
static Array(size_t) parentScopes; //Mapping between scope IDs and their parents
static Map(Symbol, Astptr) symbolTable;

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

size_t toNewScope(){
    //Largest scope ID is n-1, where n is size of parentScopes.
    //New largest scope ID will be n with curScope as parent. Size of parentScope increases as well
    if (!arrPush(size_t)(&parentScopes, curScope)){
        exit(1);
    }
    return curScope = parentScopes.size - 1;
}

size_t toPrevScope(){
    return curScope = parentScopes.elem[curScope];
}

static void insertSymbol(char_t* name, Ast* ast){
    if (!mapInsert(Symbol, Astptr)(&symbolTable, Symbol(name, curScope), ast)){
        exit(1);
    }
}

void insertVar(char_t* name, StmtVar* var){
    insertSymbol(name, &var->label);
}

void insertFunc(char_t* name, Function* func){
    insertSymbol(name, &func->label);
}

static const StmtVar* verifyVar(Ast** astptr){
    if (astptr && **astptr != astFunction){
        return (StmtVar*)(*astptr);
    }
    return NULL;
}

static const Function* verifyFunction(Ast** astptr){
    if (astptr && **astptr == astFunction){
        return (Function*)(*astptr);
    }
    return NULL;
}

//Search for a variable in only current scope
const StmtVar* findVarCurScope(char_t* name){
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, curScope));
    return verifyVar(astptr);
}

//Search for a variable from the current to the global scope
const StmtVar* findVar(char_t* name){
    size_t scopeId = curScope;
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));    
    while (astptr == NULL && scopeId != GLOBAL_SCOPE){
        scopeId = parentScopes.elem[scopeId];
        astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, scopeId));
    }
    return verifyVar(astptr);
}

//Search for a function in the global scope
const Function* findFunc(char_t* name){
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, Symbol(name, GLOBAL_SCOPE));
    return verifyFunction(astptr);
}
