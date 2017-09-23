#include "../array.h"
#include "../lexer.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"

#include "io.c"
#include "utils.c"

//Output ast to output string array for testing purposes. Put space after every output to delimit multiple nodes
void outputAst(Ast* ast){
    if (isError(ast)){   //Syntax error
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
        //TODO more print operations
        default:
            assert(0 && "Unhandled ast label"); //Nothing should fall thru the cracks
    }
}

#define test(parsefn, inputStr, expected) do { \
    ioSetup(inputStr);      \
    initLexer();            \
    initParser();           \
    outputAst(parsefn());   \
    assert(!strcmp(output, expected));  \
    disposeLexer();         \
} while(0)                  \

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

void testParseError(){
    test(parseExpr, "((sfgd) ", "ERROR "); 
    test(parseExpr, "", "ERROR "); 
    test(parseExpr, "sdg(5", "ERROR "); 
}

int main(int argc, char const *argv[])
{
    testParseBasicExpr();
    testParseCall();
    testParseBinop();
    return 0;
}