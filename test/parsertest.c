#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"
#include "semantics/symtable.h"

#include "test/utils/io.c"
#include "test/utils/assert.c"

//Output ast to output string array for testing purposes. Put space after every output to delimit multiple nodes
void outputAst(Ast* ast){
    if (ast == NULL || !checkSyntax()){   //Syntax error
        emitOut(errorstr);
        return;
    }
    switch(ast->label){
        case astExprDouble:
            emitOut("dbl:%.2Lf ", ((ExprDouble*)ast)->num); //Print numbers with 2 digit precision
            return;
        case astExprFloat:
            emitOut("flt:%.2f ", ((ExprFloat*)ast)->num);
            return;
        case astExprLong:
            emitOut("long:%lld ", ((ExprLong*)ast)->num);
            return;
        case astExprInt:
            emitOut("int:%d ", ((ExprInt*)ast)->num); //Print int
            return;
        case astExprStr:
            emitOut("str:%s ", ((ExprStr*)ast)->str); //Print string
            return;
        case astExprIdent:
            emitOut("id:%s ", ((ExprIdent*)ast)->name); //Print identifier
            return;
        case astExprCall: {
            ExprCall* expr = (ExprCall*)ast;
            emitOut("call:%s:%d ", expr->name, expr->args.size);
            for (size_t i=0; i<expr->args.size; i++){
                outputAst((Ast*)expr->args.elem[i]);
            }
            return;
        }
        case astExprUnop: {
            ExprUnop* unop = (ExprUnop*)ast;
            emitOut("%s:", stringifyToken(unop->op));
            if (!unop->leftside){
                emitOut(":");
            }
            outputAst((Ast*)unop->operand);
            return;
        }
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)ast;
            emitOut("%s ", stringifyToken(binop->op));
            outputAst((Ast*)binop->left);
            outputAst((Ast*)binop->right);
            return;
        }
        case astStmtReturn:
            emitOut("ret ");
            if (hasRetExpr((StmtReturn*)ast)){
                outputAst((Ast*)((StmtReturn*)ast)->expr);
            }
            return;
        case astStmtEmpty:
            emitOut("empty ");
            return;
        case astStmtBreak:
            emitOut("break ");
            return;
        case astStmtContinue:
            emitOut("cont ");
            return;
        case astStmtExpr:
            outputAst((Ast*)((StmtExpr*)ast)->expr);
            return;
        case astStmtBlock: {
            StmtBlock* block = (StmtBlock*)ast;
            emitOut("block:%d ", block->stmts.size);
            for (size_t i=0; i<block->stmts.size; i++){
                outputAst(block->stmts.elem[i]);
            }
            return;
        }
        case astStmtDecl:
            emitOut("dec:");
        case astStmtDef: {
            StmtVar* decl = (StmtVar*)ast;
            emitOut(stringifyType(decl->type));
            if (decl->name){
                emitOut(":%s", decl->name);
            }
            if (decl->rhs){
                emitOut(" is ");
                outputAst((Ast*)decl->rhs);
            }
            else{
                emitOut(" ");
            }
            return;
        }
        case astStmtDoWhile:
            emitOut("do:");
        case astStmtWhile: {
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            emitOut("while ");
            outputAst((Ast*)loop->condition);
            outputAst(loop->stmt);
            return;
        }
        case astFunction: {
            Function* fn = (Function*)ast;
            size_t paramCount = fn->params.size;
            emitOut("fn");
            if (fn->stmt == NULL){
                emitOut("dec");
            }
            emitOut(":%s:%s:%d ", stringifyType(fn->type), fn->name, paramCount);
            for (size_t i=0; i<paramCount; i++){
                Ast* param = fn->params.elem[i];
                assert(param->label == astStmtDef);
                outputAst(param);
            }
            if (fn->stmt){
                outputAst(fn->stmt);
            }
            return;
        }
        case astTopLevel: {
            Array(vptr)* globals = &((TopLevel*)ast)->globals;
            for (size_t i=0; i<globals->size; i++){
                outputAst(globals->elem[i]);
            }
            return;
        }
        //TODO more print operations
        default:
            assert(0 && "Unhandled ast label"); //Nothing should fall thru the cracks
    }
}

#define test(parsefn, inputStr, expected) do {\
    ioSetup(inputStr);\
    initLexer();\
    initParser();\
    initSymbolTable();\
    Ast* ast = (Ast*)parsefn();\
    outputAst(ast);\
    assertEqStr(output, expected);\
    disposeSymbolTable();\
    disposeLexer();\
    disposeAst(ast);\
    assertEqNum(checkSyntax(), 1);\
} while(0)

#define testErr(parsefn, inputStr, expected) do {\
    ioSetup(inputStr);\
    initLexer();\
    initParser();\
    initSymbolTable();\
    Ast* ast = (Ast*)parsefn();\
    assertEqStr(errorstr, expected);\
    disposeSymbolTable();\
    disposeLexer();\
    assertEqNum(checkSyntax(), 0);\
} while(0)

void testParseBasicExpr(){
    test(parseStmtOrDef, "55.55;", "dbl:55.55 ");
    test(parseStmtOrDef, "55.55f;", "flt:55.55 ");
    test(parseStmtOrDef, "78;", "int:78 ");
    test(parseStmtOrDef, "78U;", "int:78 ");
    test(parseStmtOrDef, "78LLU;", "long:78 ");
    test(parseStmtOrDef, "78Ll;", "long:78 ");
    test(parseStmtOrDef, "\"hey\";", "str:hey "); 
    test(parseStmtOrDef, "a;", "id:a "); 
    test(parseStmtOrDef, "((a));", "id:a "); 
}

void testParseCall(){
    test(parseStmtOrDef, "  omfg(5,\"\"  , var );", "call:omfg:3 int:5 str: id:var "); 
    test(parseStmtOrDef, "abaj() ;", "call:abaj:0 "); 
}

void testParseBinop(){
    test(parseStmtOrDef, "1 + 2.500 * k;", "+ int:1 * dbl:2.50 id:k "); 
    test(parseStmtOrDef, "heyo( a + b * c/d - e);", "call:heyo:1 - + id:a / * id:b id:c id:d id:e "); 
    test(parseStmtOrDef, "(a-\"lll\")/d;", "/ - id:a str:lll id:d ");
    test(parseStmtOrDef, "1 + 3 = 4 + 5*2;", "= + int:1 int:3 + int:4 * int:5 int:2 ");
    test(parseStmtOrDef, "i += 2 = 4 -= 5 *= 6 /= 7;", "+= id:i = int:2 -= int:4 *= int:5 /= int:6 int:7 ");
    test(parseStmtOrDef, "a+b+c+d;", "+ + + id:a id:b id:c id:d ");
    test(parseStmtOrDef, "a = c*b+d;", "= id:a + * id:c id:b id:d ");
}

void testParseUnop(){
    test(parseStmtOrDef, "-+-+45;", "-:-:int:45 ");
    test(parseStmtOrDef, "--++5;", "--:++:int:5 ");
    test(parseStmtOrDef, "-3++--;", "-:--::++::int:3 ");
    test(parseStmtOrDef, "4 - -5;", "- int:4 -:int:5 ");
}

void testParseStmt(){
    test(parseStmtOrDef, "return (f) ;", "ret id:f ");
    test(parseStmtOrDef, "return f() ;", "ret call:f:0 ");
    test(parseStmtOrDef, "return;", "ret ");
    test(parseStmtOrDef, " ;", "empty ");
    test(parseStmtOrDef, "break;", "break ");
    test(parseStmtOrDef, "continue;", "cont ");
    testErr(parseStmtOrDef, "break", "1:5 expected ; before end of file.\n");
    testErr(parseStmtOrDef, "continue", "1:8 expected ; before end of file.\n");
    test(parseStmtOrDef, "{}", "block:0 ");
    test(parseStmtOrDef, "{;}", "block:1 empty ");
    test(parseStmtOrDef, "{ return 5;;}", "block:2 ret int:5 empty ");
    test(parseStmtOrDef, "long long lli;", "long long:lli ");
    test(parseStmtOrDef, "int x;", "int:x ");
    test(parseStmtOrDef, "int x = 5 + 4;", "int:x is + int:5 int:4 ");
    test(parseStmtOrDef, "while (a+b){;}", "while + id:a id:b block:1 empty ");
    test(parseStmtOrDef, "do return x; while(a+b);", "do:while + id:a id:b ret id:x ");
}

void testParseFunction(){
    //This one also tests all the types so far
    test(
        parseTopLevel, "long long int main (int a, double b, long c, long long d, long int, float);",
        "fndec:long long:main:6 int:a double:b int:c long long:d int float "
    );
    test(parseTopLevel, "double k() {return 5;}", "fn:double:k:0 block:1 ret int:5 ");
    test(parseTopLevel, "unsigned char a(); signed short b(){a();}", "fndec:unsigned char:a:0 fn:short:b:0 block:1 call:a:0 ");
    test(parseTopLevel, "signed long x(unsigned short int, signed int);", "fndec:int:x:2 unsigned short int ");
    test(parseTopLevel, "void main();", "fndec:void:main:0 ");
}

void testParseError(){
    testErr(
        parseStmtOrDef, "((sfgd) ",
        "1:8 expected ) before end of file.\n1:8 expected ; before end of file.\n"
    ); 
    testErr(parseStmtOrDef, "", "1:0 expected statement before end of file.\n"); 
    testErr(parseStmtOrDef, "sdg(,", "1:4 expected expression before ,.\n");
    testErr(parseStmtOrDef, "\"wergrh\n", "2:0 expected statement before end of file.\n");
    testErr(parseStmtOrDef, "\"wergr", "1:6 expected statement before end of file.\n");
    testErr(
        parseStmtOrDef, "return k(k",
        "1:10 expected ) before end of file.\n1:10 expected ; before end of file.\n"
    );
    testErr(parseStmtOrDef, "/**/bind k;", "1:9 expected ; before identifier.\n");
    testErr(parseStmtOrDef, "/*   *", "1:6 expected statement before end of file.\n");
    testErr(parseStmtOrDef, "int 5;", "1:4 expected identifier before integer literal.\n");
    testErr(parseStmtOrDef, "signed s;", "1:7 expected type name before identifier.\n");
    testErr(parseStmtOrDef, "long x + 5", "1:7 expected = or ; before +.\n");
    testErr(parseStmtOrDef, "while 5;", "1:6 expected ( before integer literal.\n");
    testErr(parseStmtOrDef, "while (10 {}", "1:10 expected ) before {.\n");
    testErr(parseStmtOrDef, "while (a) int x;", "1:10 expected statement before keyword \"int\".\n");
    testErr(parseStmtOrDef, "do {} while(a)", "1:14 expected ; before end of file.\n");
    testErr(parseStmtOrDef, "do {} ", "1:6 expected keyword \"while\" before end of file.\n");
    testErr(parseStmtOrDef, "{ ", "1:2 expected statement before end of file.\n");

    testErr(parseTopLevel, "int Blue(a)", "1:9 expected type name before identifier.\n");
    testErr(parseTopLevel, "int Blue(long, )", "1:15 expected type name before ).\n");
    testErr(parseTopLevel, "int Blue(float f)", "1:17 expected ; or { before end of file.\n");
    testErr(parseTopLevel, "signed float a();", "1:7 expected type name before keyword \"float\".\n");
    testErr(parseTopLevel, "signed int a(unsigned);", "1:21 expected type name before ).\n");
    testErr(parseTopLevel, "dog", "1:0 expected type name before identifier.\n");
    testErr(parseTopLevel, "int stmt() return;", "1:11 expected ; or { before keyword \"return\".\n");
}

int main(int argc, char const *argv[])
{
    testParseBasicExpr();
    testParseCall();
    testParseBinop();
    testParseUnop();
    testParseStmt();
    testParseFunction();
    testParseError();
    return 0;
}