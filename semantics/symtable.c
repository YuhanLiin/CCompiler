#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "utils.h"
#include "scope/scope.h"
#include "semantics/symtable.h"

typedef Ast* Astptr;
#define KEY Symbol
#define VAL Astptr
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

static Map(Symbol, Astptr) symbolTable;

void initSymbolTable(){
    resetScopes();
    mapInit(Symbol, Astptr)(&symbolTable, 4, &hashSymbol, &eqSymbol, NULL, NULL);
}

void disposeSymbolTable(){
    mapDispose(Symbol, Astptr)(&symbolTable);
}


static void insertSymbol(char_t* name, Ast* ast){
    if (!mapInsert(Symbol, Astptr)(&symbolTable, (Symbol){name, curScope}, ast)){
        exit(1);
    }
}

void insertVar(char_t* name, StmtVar* var){
    insertSymbol(name, &var->label);
}

void insertFunc(char_t* name, Function* func){
    insertSymbol(name, &func->label);
}

//Search for a variable in only current scope
const Ast* findSymbolCurScope(char_t* name){
    Ast **astptr = mapFind(Symbol, Astptr)(&symbolTable, (Symbol){name, curScope});
    if (astptr){
        return *astptr;
    }
    return NULL;
}

//Search for a variable from the current to the global scope
const StmtVar* findVar(char_t* name){
    size_t scopeId = curScope;
    Ast** astptr = NULL;
    while (astptr == NULL) {
        astptr = mapFind(Symbol, Astptr)(&symbolTable, (Symbol){name, scopeId});
        if (scopeId == GLOBAL_SCOPE) break;
        scopeId = prevScope(scopeId);
    }
    if (astptr && **astptr != astFunction){
        return (StmtVar*)(*astptr);
    }
    return NULL;
}

//Search for a function in the global scope
const Function* findFunc(char_t* name){
    Ast** astptr = mapFind(Symbol, Astptr)(&symbolTable, (Symbol){name, GLOBAL_SCOPE});
    if (astptr && **astptr == astFunction){
        return (Function*)(*astptr);
    }
    return NULL;
}
