#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"
#include "semantics/symtable.h"

#include "test/io.c"
#include "test/utils.c"

//Output ast to output string array for testing purposes. Put space after every output to delimit multiple nodes
void outputAst(Ast* ast){
    if (ast == NULL){   //Syntax error
        outprint(errorstr);
        return;
    }
    switch(ast->label){
        case astExprDouble:
            outprint("dbl:%.2Lf ", ((ExprDouble*)ast)->num); //Print numbers with 2 digit precision
            return;
        case astExprFloat:
            outprint("flt:%.2f ", ((ExprFloat*)ast)->num);
            return;
        case astExprLong:
            outprint("long:%lld ", ((ExprLong*)ast)->num);
            return;
        case astExprInt:
            outprint("int:%d ", ((ExprInt*)ast)->num); //Print int
            return;
        case astExprStr:
            outprint("str:%s ", ((ExprStr*)ast)->str); //Print string
            return;
        case astExprIdent:
            outprint("id:%s ", ((ExprIdent*)ast)->name); //Print identifier
            return;
        case astExprCall: {
            ExprCall* expr = (ExprCall*)ast;
            outprint("call:%s:%d ", expr->name, expr->args.size);
            for (size_t i=0; i<expr->args.size; i++){
                outputAst((Ast*)expr->args.elem[i]);
            }
            return;
        }
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)ast;
            outprint("%s ", stringifyToken(binop->op));
            outputAst((Ast*)binop->left);
            outputAst((Ast*)binop->right);
            return;
        }
        case astStmtReturn:
            outprint("ret ");
            if (hasRetExpr((StmtReturn*)ast)){
                outputAst((Ast*)((StmtReturn*)ast)->expr);
            }
            return;
        case astStmtEmpty:
            outprint("empty ");
            return;
        case astStmtExpr:
            outputAst((Ast*)((StmtExpr*)ast)->expr);
            return;
        case astStmtBlock: {
            StmtBlock* block = (StmtBlock*)ast;
            outprint("block:%d ", block->stmts.size);
            for (size_t i=0; i<block->stmts.size; i++){
                outputAst(block->stmts.elem[i]);
            }
            return;
        }
        case astStmtDecl:
            outprint("dec:");
        case astStmtDef: {
            StmtVar* decl = (StmtVar*)ast;
            outprint(stringifyType(decl->type));
            if (decl->name){
                outprint(":%s", decl->name);
            }
            outprint(" ");
            return;
        }
        case astFunction: {
            Function* fn = (Function*)ast;
            size_t paramCount = fn->params.size;
            outprint("fn");
            if (fn->stmt == NULL){
                outprint("dec");
            }
            outprint(":%s:%s:%d ", stringifyType(fn->type), fn->name, paramCount);
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
    test(parseStmt, "55.55;", "dbl:55.55 ");
    test(parseStmt, "55.55f;", "flt:55.55 ");
    test(parseStmt, "78;", "int:78 ");
    test(parseStmt, "78U;", "int:78 ");
    test(parseStmt, "78LLU;", "long:78 ");
    test(parseStmt, "78Ll;", "long:78 ");
    test(parseStmt, "\"hey\";", "str:hey "); 
    test(parseStmt, "a;", "id:a "); 
    test(parseStmt, "((a));", "id:a "); 
}

void testParseCall(){
    test(parseStmt, "  omfg(5,\"\"  , var );", "call:omfg:3 int:5 str: id:var "); 
    test(parseStmt, "abaj() ;", "call:abaj:0 "); 
}

void testParseBinop(){
    test(parseStmt, "1 + 2.500 * k;", "'+' int:1 '*' dbl:2.50 id:k "); 
    test(parseStmt, "heyo( a + b * c/d - e);", "call:heyo:1 '-' '+' id:a '/' '*' id:b id:c id:d id:e "); 
    test(parseStmt, "(a-\"lll\")/d;", "'/' '-' id:a str:lll id:d "); 
}

void testParseStmt(){
    test(parseStmt, "return (f) ;", "ret id:f ");
    test(parseStmt, "return f() ;", "ret call:f:0 ");
    test(parseStmt, "return;", "ret ");
    test(parseStmt, " ;", "empty ");
    test(parseStmt, "{}", "block:0 ");
    test(parseStmt, "{ return 5;;}", "block:2 ret int:5 empty ");
    test(parseStmt, "long long lli;", "long long:lli ");
}

void testParseFunction(){
    //This one also tests all the types so far
    test(
        parseTopLevel, "long long int main (int a, double b, long c, long long d, long int, float);",
        "fndec:long long:main:6 int:a double:b int:c long long:d int float "
    );
    test(parseTopLevel, "double k() return 5;", "fn:double:k:0 ret int:5 ");
    test(parseTopLevel, "unsigned char a(); signed short b(){a();}", "fndec:unsigned char:a:0 fn:short:b:0 block:1 call:a:0 ");
    test(parseTopLevel, "signed long x(unsigned short int, signed int);", "fndec:int:x:2 unsigned short int ");
    test(parseTopLevel, "void main();", "fndec:void:main:0 ");
}

void testParseError(){
    testErr(
        parseStmt, "((sfgd) ",
        "1:8 expected ')' before end of file.\n1:8 expected ';' before end of file.\n"
    ); 
    testErr(parseStmt, "", "1:0 expected statement before end of file.\n"); 
    testErr(parseStmt, "sdg(,", "1:4 expected expression before ','.\n");
    testErr(parseStmt, "\"wergrh\n", "2:0 expected statement before end of file.\n");
    testErr(parseStmt, "\"wergr", "1:6 expected statement before end of file.\n");
    testErr(
        parseStmt, "return k(k",
        "1:10 expected ')' before end of file.\n1:10 expected ';' before end of file.\n"
    );
    testErr(parseStmt, "/**/bind k;", "1:9 expected ';' before identifier.\n");
    testErr(parseStmt, "/*   *", "1:6 expected statement before end of file.\n");
    testErr(parseStmt, "int 5;", "1:4 expected identifier before integer.\n");
    testErr(parseStmt, "signed s;", "1:7 expected type name before identifier.\n");
    testErr(parseTopLevel, "int Blue(a)", "1:9 expected type name before identifier.\n");
    testErr(parseTopLevel, "int Blue(long, )", "1:15 expected type name before ')'.\n");
    testErr(parseTopLevel, "int Blue(float f)", "1:17 expected statement before end of file.\n");
    testErr(parseTopLevel, "signed float a();", "1:7 expected type name before keyword \"float\".\n");
    testErr(parseTopLevel, "signed int a(unsigned);", "1:21 expected type name before ')'.\n");
    testErr(parseTopLevel, "dog", "1:0 expected type name before identifier.\n");
}

int main(int argc, char const *argv[])
{
    testParseBasicExpr();
    testParseCall();
    testParseBinop();
    testParseStmt();
    testParseFunction();
    testParseError();
    return 0;
}