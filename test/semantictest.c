#include "../semantic.h"
#include "../symbols.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"

#include "io.c"
#include "utils.c"

void test(void (*verifier)(Ast*), const char_t* inputStr, char expectedResult) {
    ioSetup(inputStr);
    initLexer();
    initParser();
    initSymbolTable();
    initSemantics();
    verifier(parseTopLevel());
    assert(checkSemantics() == expectedResult);
    disposeLexer();
    disposeSymbolTable();
}

void testVerifyMain(){
    test(verifyTopLevel, "int main() return 4;", 1);
    //These 2 will change in future
    test(verifyTopLevel, "int main() return 4.5;", 0);
    test(verifyTopLevel, "float main() return 44.4;", 1);
}

void testVerifyArgs(){
    test(verifyTopLevel, "int a(int a, int b, int c) return a+b+c;", 1);
    test(verifyTopLevel, "int a(int a, int b, int c) return d;", 0);
    test(verifyTopLevel, "int a(int a, int);", 1);
    test(verifyTopLevel, "int a(int a, int) return a;", 0);
    test(verifyTopLevel, "int a(int a, int a) return a;", 0);
}

int main(int argc, char const *argv[])
{
    testVerifyMain();
    testVerifyArgs();
    return 0;
}