#include "array.h"
#include "lexer/lexer.h"
#include "ast/ast.h"
#include "utils.h"
#include "semantics/semantics.h"
#include "./private.h"
#include <stdlib.h>
#include <assert.h>

//args := expr [, expr]*   Assumes args array is already initialized and empty. Return 0 for syntax error
static char parseArgs(Array(vptr) *args){
    //While comma exists consume it and keep parsing expressions
    do {
        ExprBase* arg = parseExpr();
        if (arg == NULL) return 0; //If expression cant be parsed then syntax error
        if (!arrPush(vptr)(args, arg)) exit(1); //Push arg and check for malloc failures
    }while (curTok == tokComma && (getTok() || 1));   
    return 1;
}

//primeExpr := Number | String | ( expr ) | Ident ( args? )?
static ExprBase* parsePrimaryExpr(){
    switch (curTok){
        case tokNumInt: {
            ExprInt* expr = verifyExprInt(newExprInt(lineNumberTokStart, linePosTokStart, intVal));
            getTok();
            return (ExprBase*)expr;
        }
        case tokNumUInt: {
            ExprInt* expr = verifyExprUnsignedInt(newExprInt(lineNumberTokStart, linePosTokStart, intVal));
            getTok();
            return (ExprBase*)expr;
        }
        case tokNumLong: {
            ExprLong* expr = verifyExprLong(newExprLong(lineNumberTokStart, linePosTokStart, intVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumULong: {
            ExprLong* expr = verifyExprUnsignedLong(newExprLong(lineNumberTokStart, linePosTokStart, intVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumFloat: {
            ExprFloat* expr = verifyExprFloat(newExprFloat(lineNumberTokStart, linePosTokStart, floatVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumDouble: {
            ExprDouble* expr = verifyExprDouble(newExprDouble(lineNumberTokStart, linePosTokStart, floatVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokString: {
            ExprStr* expr = newExprStr(lineNumberTokStart, linePosTokStart, toCstring(&stringBuffer));
            getTok(); //Consume string
            return (ExprBase*)expr;
        }
        case tokLParen: {
            getTok(); //Consume left paren
            ExprBase* expr = parseExpr();
            if (expr){
                //Look for right paren. If not found just pretend it's there and write to error
                if (curTok == tokRParen){
                    getTok();
                }
                else {
                    syntaxError(stringifyToken(tokRParen));
                }
                return expr;
                disposeAst(expr);
            }
            return NULL;
        }
        case tokIdent: {
            char_t* name = toCstring(&stringBuffer);
            size_t nameline = lineNumberTokStart;
            size_t namepos = linePosTokStart;
            getTok(); //consume identifier
            //If bracket follows then its a function call
            if (curTok == tokLParen){  
                getTok(); //Consume left paren
                ExprCall* call = newExprCall(nameline, namepos, name);
                //If following brackets are not empty, attempt to parse 1 or more args.
                if (curTok == tokRParen){
                    getTok(); //Consume rb
                    return (ExprBase*)verifyExprCall(call);
                } 
                else{
                    //Attempt to parse arguments. It will throw if unsuccessful so no need to handle it here
                    if (parseArgs(&call->args)){
                        if (curTok == tokRParen){
                            getTok(); //Consume rb
                        }
                        else{
                            syntaxError(stringifyToken(tokRParen));
                        }
                        return (ExprBase*)verifyExprCall(call);
                    }
                }
                arrDispose(vptr)(&call->args);
                free(call);
            }
            //Otherwise its a normal identifier
            else {
                return (ExprBase*)verifyExprIdent(newExprIdent(nameline, namepos, name));
            }
            free(name);
            return NULL;
        }
        default:
            syntaxError("expression");
            return NULL;
    }
}

static ExprBase* parseRightUnopExpr(){
    ExprBase* expr = parsePrimaryExpr();
    //Keep binding unary operators to the expression. Return when there is none left
    while (expr){
        size_t opLine = lineNumber;
        size_t opPos = linePos; 
        switch(curTok){
            case tokDec:
            case tokInc:
                expr = (ExprBase*)verifyExprRightUnop(newExprRightUnop(opLine, opPos, expr, curTok));
                getTok();
                break;
            default:
                return expr;
        }
    }
    return NULL;
}

static ExprBase* parseLeftUnopExpr(){
    switch(curTok){
        //All single-token unary operators go here
        case tokDec:
        case tokInc:
        case tokMinus: {
            size_t opLine = lineNumber;
            size_t opPos = linePos;
            Token op = curTok;
            getTok();
            ExprBase* operand = parseLeftUnopExpr();
            if (operand){
                return (ExprBase*)verifyExprLeftUnop(newExprLeftUnop(opLine, opPos, op, operand));
            }
            return NULL;
        }
        //Plus operator means nothing, so just skip it
        case tokPlus:
            getTok(); 
            return parseLeftUnopExpr();
        default: 
            return parseRightUnopExpr();
    }
}

static char isRightAssociative(Token op){
    switch (op){
        case tokPlusAssign:
        case tokMinusAssign:
        case tokAssign:
            return 1;
    }
    return 0;
}

//Return precedence of all binop tokens, 0 if token isnt operator
static int operatorPrec(Token op){
    switch (op){
        case tokPlusAssign:
        case tokMinusAssign:
        case tokAssign:
            return 1;
        case tokPlus:
        case tokMinus:
            return 10;
        case tokMulti:
        case tokDiv:
            return 11;
    }
    return 0;
}

//Precedence climbing algorithm for binops. Constructs lhs from subsequent terms. Will not free lhs. Returns updated lhs or error
//exprBinop := [ [+-/*] primeExpr]*
static ExprBase* parseBinopExpr(ExprBase* lhs, int minPrec){
    int prec;
    ExprBase* rhs = NULL;
    Token op;
    //Will first consume any higher/equal precedence binop
    while ((prec = operatorPrec(op = curTok)) >= minPrec){
        size_t opLine = lineNumber;
        size_t opPos = linePos;
        getTok(); //Consume binop
        //Attempt to parse 1st atom of rhs expression
        if ((rhs = parseLeftUnopExpr()) == NULL) return rhs;
        //While next binop is of higher precedence (or equal for right assiciative ops), accumulate expression into rhs.
        while (operatorPrec(curTok) > prec || isRightAssociative(op) && operatorPrec(curTok) == prec){
            //Attempt to parse subsequent atoms at a precedece equal to current binop
            ExprBase* newRhs = parseBinopExpr(rhs, operatorPrec(curTok));
            if (newRhs == NULL){
                disposeAst(rhs); //Responsible for delete rhs, which was created in this function
                return NULL;
            }
            rhs = newRhs;
        }
        //After rhs has been fully built, merge it with lhs and then continue
        lhs = (ExprBase*)verifyExprBinop(newExprBinop(opLine, opPos, op, lhs, rhs));
    }
    return lhs;
}

//expr := primeExpr exprBinop
ExprBase* parseExpr(){
    ExprBase* lhs = parseLeftUnopExpr(); //Parse the 1st primary expression
    if (lhs == NULL) return lhs; 
    ExprBase* expr = parseBinopExpr(lhs, 1); //Parse all following binops
    if (expr == NULL){
        disposeAst(lhs);
    }
    return expr;
}