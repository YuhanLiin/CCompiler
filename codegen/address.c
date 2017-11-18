#include "codegen/address.h"
#include "utils.h"
#include "scope/scope.h"
#include <stdint.h>


#define KEY Symbol
#define VAL Address
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

static Map(Symbol, Address) addressTable;

void initAddrTable(){
    resetScopes();
    mapInit(Symbol, Address)(&addressTable, 4, &hashSymbol, &eqSymbol, NULL, NULL);
}

void disposeAddrTable(){
    mapDispose(Symbol, Address)(&addressTable);
}

void insertAddress(char_t* name, Address addr){
    if (!mapInsert(Symbol, Address)(&addressTable, (Symbol){name, curScope}, addr)){
        exit(1);
    }
}

Address findAddress(char_t* name){
    size_t scopeId = curScope;
    Address* addrptr = NULL;
    while (addrptr == NULL){
        addrptr = mapFind(Symbol, Address)(&addressTable, (Symbol){name, scopeId});
        if (scopeId == GLOBAL_SCOPE) break;
        scopeId = prevScope(scopeId);
    }
    return *addrptr;
}

Address registerAddress(Register reg){
    return (Address){registerMode, {.reg = reg}};
}
Address symbolAddress(char_t *symbol){
    return (Address){symbolMode, {.symbol = symbol}};
}
Address numberAddress(uint64_t num){
    return (Address){numberMode, {.num = num}};
}
Address indirectAddress(int64_t offset, Register reg){
    return (Address){indirectMode, {.indirect = {.offset = offset, .reg = reg}}};
}