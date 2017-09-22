#include "../array.h"
#include "../lexer.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"

#include "io.c"
#include "utils.c"

Ast* ast;

//Output ast to output string array for testing purposes. Put space after every output to delimit multiple nodes
void outputAst(Ast* ast){
    if (isError(ast)){   //Syntax error
        outprint("ERROR ");
        return;
    }
    switch(*ast){
        case astExprNum:
            outprint("num:%.2Lf ", ((ExprNum*)ast)->num); //Print numbers with 2 digit precision
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
    disposeLexer();         \
    ioSetup(inputStr);      \
    outputAst(parsefn());   \
    assert(!strcmp(output, expected));  \
    initLexer();            \
    initParser();           \
} while(0)                  \

void testParseBasicExpr(){
    setup("55.55");
    outputAst(parseExpr());
    assert(!strcmp(output, "num:55.55 "));
    setup("\"hey\"");
    outputAst(parseExpr());
    assert(!strcmp(output, "str:hey "));
    setup("a");
    outputAst(parseExpr());
    assert(!strcmp(output, "id:a "));
    setup("((a))");
    outputAst(parseExpr());
    assert(!strcmp(output, "id:a "));
}

void testParseCall(){
    setup("  omfg(5,\"\"  , var )");
    outputAst(parseExpr());
    assert(!strcmp(output, "call:omfg:3 num:5.00 str: id:var "));
    setup("abaj()");
    outputAst(parseExpr());
    assert(!strcmp(output, "call:abaj:0 "));
}

void testParseBinop(){
    setup("1 + 2 * k");
    outputAst(parseExpr());
    assert(!strcmp(output, "+ num:1.00 * num:2.00 id:k "));
    setup("heyo( a + b * c/d - e)");
    outputAst(parseExpr());
    assert(!strcmp(output, "call:heyo:1 - + id:a / * id:b id:c id:d id:e "));
    setup("(a-\"lll\")/d");
    outputAst(parseExpr());
    assert(!strcmp(output, "/ - id:a str:lll id:d "));
}

void testParseError(){
    setup("((sfgd) ");
    outputAst(parseExpr());
    assert(!strcmp(output, "ERROR "));
    setup("");
    outputAst(parseExpr());
    assert(!strcmp(output, "ERROR "));
    setup("sdg(5");
    outputAst(parseExpr());
    assert(!strcmp(output, "ERROR "));
}