#include "semantics/symtable.h"
#include "semantics/semantics.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"

#include "./io.c"
#include "./utils.c"

#define test(inputStr) do {\
    ioSetup(inputStr);\
    initLexer();\
    initParser();\
    initSymbolTable();\
    parseTopLevel();\
    assertEqNum(checkSemantics(), 1);\
    disposeLexer();\
    disposeSymbolTable();\
} while(0)

#define testErr(inputStr, expected) do{\
    ioSetup(inputStr);\
    initLexer();\
    initParser();\
    initSymbolTable();\
    parseTopLevel();\
    assertEqNum(checkSemantics(), 0);\
    assertEqStr(errorstr, expected);\
    disposeLexer();\
    disposeSymbolTable();\
} while(0)

#define pls "pls"

void testVerifyMain(){
    test("int main() return 4;");
    //These 2 will change in future
    testErr("int main() return 4.5;", "1:18 no way to convert value type 'float' to return type 'int'.");
    test("float main() return 44.4;");
}

void testVerifyArgs(){
    test("int a(int a, int b, int c) return a+b+c;");
    testErr("int a(int a, int b, int c) return d;", pls);
    test("int a(int a, int);");
    testErr("int a(int a, int) return a;", pls);
    testErr("int a(int a, int a) return a;", pls);
}

void testFuncDuplication(){
    testErr("int a(); float a();", pls);
    testErr("int a(); int a(float b);", pls);
    test("int a(int s); int a(int s){}");
    testErr("int a(int s){} int a(int s);", pls);

    test("int a(int s){} int s();");
    test("int b(); float k(int b){}");
    //TODO test global vars
}

void testCall(){
    test("int a(); int b(){a();}");
    testErr("int b(){ wtf();}", pls);
    test("int r(int a, long b){ return r(a, b);}");
    testErr("int a(); int b(float f){ a(f);}", pls);
    testErr("int a(long l); int b(float f){ a(f);}", pls);
}

int main(int argc, char const *argv[])
{
    testVerifyMain();
    testVerifyArgs();
    testFuncDuplication();
    testCall();
    return 0;
}