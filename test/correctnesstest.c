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
    assertNotEqNum(parseTopLevel(), NULL);\
    assertEqNum(checkSemantics(), 1);\
    disposeLexer();\
    disposeSymbolTable();\
} while(0)

#define testErr(inputStr, expected) do{\
    ioSetup(inputStr);\
    initLexer();\
    initParser();\
    initSymbolTable();\
    assertNotEqNum(parseTopLevel(), NULL);\
    assertEqNum(checkSemantics(), 0);\
    assertEqStr(errorstr, expected);\
    disposeLexer();\
    disposeSymbolTable();\
} while(0)

static void testReturn(){
    test("int main() {return 4;}");
    test("int main() {return 4.5;}");
    test("int main() {return;}");
    test("void main() {return;}");
    test("void noreturn(){}");
    test("int noreturn(){}");
    testErr("void main() {return 5;}", "1:13 cannot return a value from a void function.\n");
    testErr(
        "void main(){ return undefined;}", 
        "1:20 attempting to reference undeclared variable 'undefined'.\n1:13 cannot return a value from a void function.\n"
    );
}

static void testParams(){
    test("int a(int a, int b, int c) {return a+b+c;}");
    testErr("int a(int a, int b, int c) {return d;}", "1:35 attempting to reference undeclared variable 'd'.\n");
    test("int a(int a, int);");
    testErr("int a(int a, int) {return a;}", "1:16 nameless parameter in function definition.\n");
    testErr("int a(int a, int a){ return a;}", "1:16 parameter 'a' has already been defined.\n");
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
    test("int neg() {return- - + 4;}");
    test("float x(int o, int b) {o+b*3.2/43-5;}");
    testErr(
        "void v() {v()+v()-v();}", 
        "1:10 cannot use a void-returning function call as an expression.\n"
        "1:14 cannot use a void-returning function call as an expression.\n"
        "1:18 cannot use a void-returning function call as an expression.\n"
    );
    testErr(
        "void v() {a + b * nopropagate;}",
        "1:10 attempting to reference undeclared variable 'a'.\n"
        "1:14 attempting to reference undeclared variable 'b'.\n"
        "1:18 attempting to reference undeclared variable 'nopropagate'.\n"
    );
}

static void testAssignment(){
    test("int x(int a){a = 4;}");
    test("int x(int a){a += 4;}");
    test("int x(int a){a -= 4;}");
    test("int x(int a){a /= 4;}");
    test("int x(int a){a *= 4;}");
    testErr("int x(int a){54 = 4;}", "1:13 lvalue required on left of assignment.\n");
}

static void testUnop(){
    testErr("int bad(){return --5 + ++5;}", "1:17 lvalue required as operand of --.\n1:23 lvalue required as operand of ++.\n");
    testErr("int bad(int l) {return --l--;}", "1:23 lvalue required as operand of --.\n");
    test("int good(int l) {l++; ++l; --l; l--;}");
}

static void testDefineVar(){
    test("void v(){int x = 5;}");
    test("void v(){int x ;}");
    testErr("void v(){int x = v();}", "1:17 cannot use a void-returning function call as an expression.\n");
    testErr("void v(short x){int x;}", "1:20 variable 'x' has already been defined.\n");
}

static void testLoops(){
    test("int m(){while(1) break;}");
    test("int m(){do continue; while(0);}");
    test("int m(){while(1){while(1) break; continue;}}");
    testErr(
        "int m(){break; continue;}",
        "8:23 break statement placed outside of loop or switch block.\n"
        "15:25 continue statement placed outside of loop.\n"
    );
}

int main(int argc, char const *argv[])
{
    testReturn();
    testParams();
    testFuncDuplication();
    testCall();
    testArithmetic();
    testAssignment();
    testUnop();
    testDefineVar();
    testLoops();
    return 0;
}