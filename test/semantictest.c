#include "semantics/semantics.h"
#include "scope/scope.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"
#include "semantics/symtable.h"

#include "./io.c"
#include "./utils.c"

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
    //TODO test global vars
}

void testCall(){
    test("int a(); int b(){a();}", 1);
    test("int b(){ wtf();}", 0);
    test("int r(int a, long b){ return r(a, b);}", 1);
    test("int a(); int b(float f){ a(f);}", 0);
    test("int a(long l); int b(float f){ a(f);}", 0);
}

int main(int argc, char const *argv[])
{
    testVerifyMain();
    testVerifyArgs();
    testFuncDuplication();
    testCall();
    return 0;
}