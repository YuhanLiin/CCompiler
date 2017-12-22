#include "scope/scope.h"
#include "test/utils/assert.h"

static void testInitReset(){
    initScopes();
    assertEqNum(curScope, GLOBAL_SCOPE);
    curScope = 5;
    resetScopes();
    assertEqNum(curScope, GLOBAL_SCOPE);
    disposeScopes();
}

static void testEqSymbol(){
    assertn0(eqSymbol((Symbol){"Dog", 3}, (Symbol){"Dog", 3}));
    assert0(eqSymbol((Symbol){"Dog", 4}, (Symbol){"Dog", 3}));
    assert0(eqSymbol((Symbol){"Dog", 3}, (Symbol){"Doggy", 3}));
}

static void testHashSymbol(){
    assertEqNum(hashSymbol((Symbol){"Dog", 3}), hashSymbol((Symbol){"Dog", 3}));
    assertNotEqNum(hashSymbol((Symbol){"Dog", 4}), hashSymbol((Symbol){"Dog", 3}));
    assertNotEqNum(hashSymbol((Symbol){"Dog", 3}), hashSymbol((Symbol){"Doggy", 3}));
}

#define checkNewScope(scopeIncrease) do{\
    assertEqNum(toNewScope(), curScope);\
    assertEqNum(curScope, GLOBAL_SCOPE + (scopeIncrease));\
} while (0)
#define checkPrevScope(prevScopeIncrease) do{\
    assertEqNum(toPrevScope(), curScope);\
    assertEqNum(curScope, GLOBAL_SCOPE + (prevScopeIncrease));\
}while(0)

static void testNewPrevScope(){
    initScopes();
    for (size_t i=1; i<6; i++){
        checkNewScope(i);
    }
    for (size_t i=1; i<6; i++){
        assertEqNum(prevScope(i), i-1);
    }
    for (size_t i=5; i>0; i--){
        toPrevScope((i-1));
    }
    disposeScopes();
}

int main(){
    testInitReset();
    testHashSymbol();
    testEqSymbol();
    testNewPrevScope();
    return 0;
}