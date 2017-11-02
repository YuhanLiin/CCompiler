#include "../semantic.h"
#include "../symbols.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"
#include "../symtable.h"

#include "io.c"
#include "utils.c"

void test(const char_t* inputStr, char expectedResult) {
    ioSetup(inputStr);
    initLexer();
    initParser();
    initScopes();
    initSymbolTable();
    initSemantics();
    verifyTopLevel(parseTopLevel());
    assert(checkSemantics() == expectedResult);
    disposeLexer();
    disposeSymbolTable();
    disposeScopes();
}

void testVerifyMain(){
    test("int main() return 4;", 1);
    //These 2 will change in future
    test("int main() return 4.5;", 0);
    test("float main() return 44.4;", 1);
}

void testVerifyArgs(){
    test("int a(int a, int b, int c) return a+b+c;", 1);
    test("int a(int a, int b, int c) return d;", 0);
    test("int a(int a, int);", 1);
    test("int a(int a, int) return a;", 0);
    test("int a(int a, int a) return a;", 0);
}

void testFuncDuplication(){
    test("int a(); float a();", 0);
    test("int a(); int a(float b);", 0);
    test("int a(int s); int a(int s){}", 1);
    test("int a(int s){} int a(int s);", 0);

    test("int a(int s){} int s();", 1);
    test("int b(); float k(int b){}", 1);
    //TODO test global vars and function calls
}

int main(int argc, char const *argv[])
{
    testVerifyMain();
    testVerifyArgs();
    testFuncDuplication();
    return 0;
}