#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/ast.h"
#include "parser/parser.h"

#include "./io.c"
#include "./utils.c"

//Output ast to output string array for testing purposes. Put space after every output to delimit multiple nodes
void outputAst(Ast* ast){
    if (ast == NULL){   //Syntax error
        outprint("ERROR ");
        return;
    }
    switch(*ast){
        case astExprDouble:
            outprint("dbl:%.2Lf ", ((ExprDouble*)ast)->num); //Print numbers with 2 digit precision
            return;
        case astExprInt:
            outprint("int:%lld ", ((ExprInt*)ast)->num); //Print int
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
            outprint("%s ", stringifyOp(binop->op));
            outputAst((Ast*)binop->left);
            outputAst((Ast*)binop->right);
            return;
        }
        case astStmtReturn:
            outprint("ret ");
            outputAst((Ast*)((StmtReturn*)ast)->expr);
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
                assert(*param == astStmtDef);
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

void test(Ast* (*parsefn)(), const char_t* inputStr, const char_t* expected){ 
    ioSetup(inputStr);      
    initLexer();           
    initParser();
    Ast* ast = parsefn();          
    outputAst(ast);
    assert(!strcmp(output, expected));  
    disposeLexer();
    disposeAst(ast);
    assert(checkParse());
}                 

void testErr(Ast* (*parsefn)(), const char_t* inputStr, const char_t* expected){ 
    ioSetup(inputStr);      
    initLexer();            
    initParser();
    Ast* ast = parsefn();
    assert(!strcmp(errorstr, expected));  
    disposeLexer();
    assert(!checkParse());
} 

void testParseBasicExpr(){
    test(parseStmt, "55.55;", "dbl:55.55 ");
    test(parseStmt, "78;", "int:78 ");
    test(parseStmt, "\"hey\";", "str:hey "); 
    test(parseStmt, "a;", "id:a "); 
    test(parseStmt, "((a));", "id:a "); 
}

void testParseCall(){
    test(parseStmt, "  omfg(5,\"\"  , var );", "call:omfg:3 int:5 str: id:var "); 
    test(parseStmt, "abaj() ;", "call:abaj:0 "); 
}

void testParseBinop(){
    test(parseStmt, "1 + 2.500 * k;", "+ int:1 * dbl:2.50 id:k "); 
    test(parseStmt, "heyo( a + b * c/d - e);", "call:heyo:1 - + id:a / * id:b id:c id:d id:e "); 
    test(parseStmt, "(a-\"lll\")/d;", "/ - id:a str:lll id:d "); 
}

void testParseStmt(){
    test(parseStmt, "return (f) ;", "ret id:f ");
    test(parseStmt, "return f() ;", "ret call:f:0 ");
    test(parseStmt, " ;", "empty ");
    test(parseStmt, "{}", "block:0 ");
    test(parseStmt, "{ return 5;;}", "block:2 ret int:5 empty ");
    test(parseStmt, "long long lli;", "i64:lli ");
}

void testParseFunction(){
    //This one also tests all the types so far
    test(
        parseTopLevel, "long long int main (int a, double b, long c, long long d, long int, float);",
        "fndec:i64:main:6 i32:a f64:b i32:c i64:d i32 f32 "
    );
    test(parseTopLevel, "double k() return 5;", "fn:f64:k:0 ret int:5 ");
    test(parseTopLevel, "int a(); int b(){a();}", "fndec:i32:a:0 fn:i32:b:0 block:1 call:a:0 ");
}

void testParseError(){
    testErr(
        parseStmt, "((sfgd) ",
        "On line 1, position 8, expected ')', but found end of file.\nOn line 1, position 8, expected ';', but found end of file.\n"
    ); 
    testErr(parseStmt, "", "On line 1, position 0, expected statement, but found end of file.\n"); 
    testErr(parseStmt, "sdg(,", "On line 1, position 5, expected expression, but found ','.\n");
    testErr(parseStmt, "\"wergrh\n", "On line 2, position 0, expected statement, but found end of file.\n");
    testErr(parseStmt, "\"wergr", "On line 1, position 6, expected statement, but found end of file.\n");
    testErr(
        parseStmt, "return k(k",
        "On line 1, position 10, expected ')', but found end of file.\nOn line 1, position 11, expected ';', but found end of file.\n"
    );
    testErr(parseStmt, "/**/bind k;", "On line 1, position 9, expected ';', but found identifier.\n");
    testErr(parseStmt, "/*   *", "On line 1, position 6, expected statement, but found end of file.\n");
    testErr(parseStmt, "int 5;", "On line 1, position 4, expected identifier, but found integer.\n");
    testErr(parseTopLevel, "int Blue(a)", "On line 1, position 9, expected type name, but found identifier.\n");
    testErr(parseTopLevel, "int Blue(long, )", "On line 1, position 15, expected type name, but found ')'.\n");
    testErr(parseTopLevel, "int Blue(float f)", "On line 1, position 17, expected statement, but found end of file.\n");
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