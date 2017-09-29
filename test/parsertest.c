#include "../array.h"
#include "../lexer.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"

#include "io.c"
#include "utils.c"

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
                outputAst(expr->args.elem[i]);
            }
            return;
        }
        case astExprBinop: {
            ExprBinop* binop = (ExprBinop*)ast;
            outprint("%s ", stringifyOp(binop->op));
            outputAst(binop->left);
            outputAst(binop->right);
            return;
        }
        case astStmtReturn:
            outprint("ret ");
            outputAst(((StmtReturn*)ast)->expr);
            return;
        case astFunction: {
            Function* fn = (Function*)ast;
            assert (fn->paramTypes.size == fn->paramNames.size);
            size_t paramCount = fn->paramTypes.size;
            outprint("fn");
            if (fn->stmt == NULL){
                outprint("dec");
            }
            outprint(":%s:%s:%d ", stringifyType(fn->type), fn->name, paramCount);
            for (size_t i=0; i<paramCount; i++){
                Type type = fn->paramTypes.elem[i];
                char_t* name = fn->paramNames.elem[i];
                outprint("%s:%s ", stringifyType(type), name);
            }
            if (fn->stmt){
                outputAst(fn->stmt);
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
}                 

void testErr(Ast* (*parsefn)(), const char_t* inputStr, const char_t* expected){ 
    ioSetup(inputStr);      
    initLexer();            
    initParser();
    Ast* ast = parsefn();
    assert(!strcmp(errorstr, expected));  
    disposeLexer();
} 

void testParseBasicExpr(){
    test(parseExpr, "55.55", "dbl:55.55 ");
    test(parseExpr, "78", "int:78 ");
    test(parseExpr, "\"hey\"", "str:hey "); 
    test(parseExpr, "a", "id:a "); 
    test(parseExpr, "((a))", "id:a "); 
}

void testParseCall(){
    test(parseExpr, "  omfg(5,\"\"  , var )", "call:omfg:3 int:5 str: id:var "); 
    test(parseExpr, "abaj()", "call:abaj:0 "); 
}

void testParseBinop(){
    test(parseExpr, "1 + 2.500 * k", "+ int:1 * dbl:2.50 id:k "); 
    test(parseExpr, "heyo( a + b * c/d - e)", "call:heyo:1 - + id:a / * id:b id:c id:d id:e "); 
    test(parseExpr, "(a-\"lll\")/d", "/ - id:a str:lll id:d "); 
}

void testParseStmt(){
    test(parseStmt, "return (f) ;", "ret id:f ");
    test(parseStmt, "return f() ;", "ret call:f:0 ");
}

void testParseFunction(){
    //This one also tests all the types so far
    test(
        parseTopLevel, "long long int main (int a, double b, long c, long long d, long int e, float f);",
        "fndec:i64:main:6 i32:a f64:b i32:c i64:d i32:e f32:f "
    );
    test(parseTopLevel, "double k() return 5;", "fn:f64:k:0 ret int:5 ");
}

void testParseError(){
    testErr(parseExpr, "((sfgd) ", "On line 1, position 8, expected ')', but found end of file.\n"); 
    testErr(parseExpr, "", "On line 1, position 0, expected expression, but found end of file.\n"); 
    testErr(parseExpr, "sdg(,", "On line 1, position 4, expected expression, but found ','.\n");
    testErr(parseExpr, "\"wergrh\n", "On line 2, position 0, expected expression, but found end of file.\n");
    testErr(parseExpr, "\"wergr", "On line 1, position 6, expected expression, but found end of file.\n");
    testErr(
        parseStmt, "return k(k",
        "On line 1, position 10, expected ')', but found end of file.\nOn line 1, position 10, expected ';', but found end of file.\n"
    );
    testErr(parseStmt, "/**/bind k;", "On line 1, position 4, expected statement, but found identifier.\n");
    testErr(parseStmt, "/*   *", "On line 1, position 6, expected statement, but found end of file.\n");
    testErr(parseTopLevel, "int Blue(a)", "On line 1, position 9, expected type name, but found identifier.\n");
    testErr(parseTopLevel, "int Blue(long, )", "On line 1, position 13, expected identifier, but found ','.\n");
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