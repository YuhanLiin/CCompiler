#include "semantics/symtable.h"
#include "semantics/semantics.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"

#include "test/io.c"
#include "test/utils.c"

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

static void testReturn(){
    test("int main() return 4;");
    test("int main() return 4.5;");
    test("int main() return;");
    test("void main() return;");
    testErr("void main() return 5;", "1:12 cannot return a value from a void function.\n");
    testErr(
        "void main() return undefined;", 
        "1:19 attempting to reference undeclared variable 'undefined'.\n1:12 cannot return a value from a void function.\n"
    );
}

static void testParams(){
    test("int a(int a, int b, int c) return a+b+c;");
    testErr("int a(int a, int b, int c) return d;", "1:34 attempting to reference undeclared variable 'd'.\n");
    test("int a(int a, int);");
    testErr("int a(int a, int) return a;", "1:16 nameless parameter in function definition.\n");
    testErr("int a(int a, int a) return a;", "1:16 parameter 'a' has already been defined.\n");
    testErr("int a(int a, void b);", "1:17 function parameters cannot have type void.\n");
}

static void testFuncDuplication(){
    testErr("int a(); float a();", "1:15 definition of 'a' does not match its previous declaration.\n");
    testErr("int a(); int a(float b);", "1:13 definition of 'a' does not match its previous declaration.\n");
    test("int a(int s); int a(int s){}");
    testErr("int a(int s){} int a(int s);", "1:19 function name 'a' has already been defined.\n");

    test("int a(int s){} int s();");
    test("int b(); float k(int b){}");
    //TODO test global vars
}

static void testCall(){
    test("int a(); int b(){a();}");
    testErr("int b(){ wtf();}", "1:9 attempting to call undeclared function 'wtf'.\n");
    test("int r(int a, long b){ return r(a, b);}");
    testErr("int a(); int b(float f){ a(f);}", "1:25 wrong number of arguments for calling function 'a'.\n");
}

static void testArithmetic(){
    test("float x(int o, int b) o+b*3.2/43-5;");
    testErr(
        "void v() v()+v()-v();", 
        "1:9 cannot use a void-returning function call as an expression.\n"
        "1:13 cannot use a void-returning function call as an expression.\n"
        "1:17 cannot use a void-returning function call as an expression.\n"
    );
    testErr(
        "void v() a + b * nopropagate;",
        "1:9 attempting to reference undeclared variable 'a'.\n"
        "1:13 attempting to reference undeclared variable 'b'.\n"
        "1:17 attempting to reference undeclared variable 'nopropagate'.\n"
    );
}

int main(int argc, char const *argv[])
{
    testReturn();
    testParams();
    testFuncDuplication();
    testCall();
    testArithmetic();
    return 0;
}