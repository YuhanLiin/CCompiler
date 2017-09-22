#include "array.h"
#include "lexer.h"
#include "ast.h"
#include "utils.h"
#include "parser.h"
#include "semantic.h"
#include <stdlib.h>
#include <assert.h>

#define NewAst(T, ptrname, ...) New(T, ptrname, 1) *ptrname = T(__VA_ARGS__);

Token curTok;  //Lookahead token. Global lexer values correspond to this token
static Token getTok(){  //Updates lookahead token
    return curTok = lexToken();
}

char isError(Ast* p){ //Checks if ptr is a SyntaxError sentinel value. Returning true means said ptr holds no resource
    return (p == SyntaxError);
} 

void initParser(){
    getTok();
}

//Allocate and return new cstring from a char array
static char_t* toCstring(const Array(char_t)* str){
    New(char_t, cstr, str->size+1)
    memcpy(cstr, str->elem, sizeof(char_t)*(str->size+1));
    cstr[str->size] = 0;
    return cstr;
}

//IMPORTANT: Extra unprocessed lookahead will always be present at start and end of every parser function call
//Return 0 for syntax error and stop processing tokens

//Attempt to parse and return a built in type
static Type parseType(){
    //TODO add signed/unsigned support
    switch (curTok){
        case tokInt:
            getTok();
            return typInt32;
        case tokFloat:
            getTok();
            return typFloat32;
        case tokDouble:
            getTok();
            return typFloat64;
        case tokLong:
            getTok();
            if (curTok == tokInt){
                getTok();
                return typInt32;
            }
            else if (curTok == tokLong){
                getTok();
                if (curTok == tokInt){
                    getTok();
                }
                return typInt64;
            }
            else{
                return typInt32;
            }
        default:
            return typNone;
    }
}
//args := expr [, expr]*   Assumes args array is already initialized and empty. Return 0 for syntax error
static char parseArgs(Array(vptr) *args){
    //While comma exists consume it and keep parsing expressions
    do {    
        Ast* arg = parseExpr();
        if (isError(arg)) return 0; //If expression cant be parsed then syntax error
        if (!pushArr(vptr)(args, arg)) exit(1); //Push arg and check for malloc failures
    }while (curTok == tokComma && (getTok() || 1));   
    return 1;
}

//primeExpr := Number | String | ( expr ) | Ident ( args? )?
static Ast* parsePrimaryExpr(){
    switch (curTok){
        case tokInt: {
            NewAst(ExprInt, expr, intVal)
            getTok(); //Consume number
            return (Ast*)expr;
        }
        case tokDouble: {
            NewAst(ExprDouble, expr, floatVal)
            getTok(); //Consume number
            return (Ast*)expr;
        }
        case tokString: {
            NewAst(ExprStr, expr, toCstring(&stringBuffer))
            getTok(); //Consume string
            return (Ast*)expr;
        }
        case tokLParen: {
            getTok(); //Consume left paren
            Ast* expr = parseExpr();
            if (expr){
                //Look for right paren. Clean up expression node if it's not found
                if (curTok == tokRParen){
                    getTok();
                    return expr;
                }
                disposeAst(expr);
            }
            return SyntaxError;
        }
        case tokIdent: {
            char_t* name = toCstring(&stringBuffer);
            getTok(); //consume identifier
            //If bracket follows then its a function call
            if (curTok == tokLParen){  
                getTok(); //Consume left paren
                NewAst(ExprCall, call, name)
                //Initialize args array
                initArr(vptr)(&call->args, 0, NULL, &disposeAst);
                //If following brackets are not empty, attempt to parse 1 or more args.
                if (curTok != tokRParen){
                    if (parseArgs(&call->args)){
                        if (curTok == tokRParen){
                            getTok(); //Consume rb
                            return call;
                        }
                    }
                } 
                else{
                    getTok();
                    return call;
                }
                disposeArr(vptr)(&call->args);
                disposeAst(call);
            }
            //Otherwise its a normal identifier
            else {
                NewAst(ExprIdent, ident, name)
                return ident;
            }
            free(name);
            return SyntaxError;
        }
        default:
            return SyntaxError;
    }
}

//Return precedence of all operator tokens, 0 if token isnt operator
static int operatorPrec(Token op){
    switch (op){
        case tokPlus: return 1;
        case tokMinus: return 1;
        case tokMulti: return 2;
        case tokDiv: return 2;
        default: return 0;
    }
}

//Precedence climbing algorithm for binops. Constructs lhs from subsequent terms. Will not free lhs. Returns updated lhs or error
//exprBinop := [ [+-/*] primeExpr]*
static Ast* parseBinopExpr(Ast* lhs, int minPrec){
    int prec;
    Ast* rhs = NULL;
    Token op;
    //Will first consume any higher/equal precedence binop. Afterwards, due to inner loop, this outer loop will only consume equal precedence binops
    while (operatorPrec(op = curTok) >= minPrec){
        getTok(); //Consume binop
        //Attempt to parse 1st atom of rhs expression
        if (isError(rhs = parsePrimaryExpr())) return rhs;
        //While next binop is of higher precedence, accumulate expression into rhs
        while ((prec = operatorPrec(curTok)) > minPrec){
            Ast* newRhs = parseBinopExpr(rhs, prec);
            //Attempt to parse subsequent atoms at a precedece equal to current binop
            if (isError(newRhs)){
                free(rhs); //Responsible for delete rhs, which was created in this scope
                return newRhs;
            }
            rhs = newRhs;
        }
        //After rhs has been fully built, merge it with lhs and then continue
        NewAst(ExprBinop, newLhs, op, lhs, rhs)
        lhs = (Ast*)newLhs;
    }
    return lhs;
}

//expr := primeExpr exprBinop
Ast* parseExpr(){
    Ast* lhs = parsePrimaryExpr(); //Parse the 1st primary expression
    if (isError(lhs)) return lhs; 
    Ast* expr = parseBinopExpr(lhs, 1); //Parse all follwing binops
    if (isError(expr)){
        free(lhs);
        return expr;
    }
    return expr;
}

static Ast* parseStmt(){
    if (curTok == tokReturn){
        getTok(); //Consume return
        Ast* expr = parseExpr();
        if (expr){
            if (curTok == tokSemicolon){
                getTok(); //Consume semicolon
                NewAst(StmtReturn, stmt, expr)
                return stmt;
            }
        }
        free(expr);
        return SyntaxError;
    }
    else{
        //TODO handle other statements
        return SyntaxError;
    }
}

static char parseParams(Array(Type) *types, Array(vptr) *names){
    //While comma exists, consume it (the getTok() call) and keep parsing identifiers
    do {    
        Type type = parseType();
        //Each param must consist of a type and a name
        if (type == typNone || curTok != tokIdent){ 
            return 0;
        }
        if (!pushArr(vptr)(params, toCstring(stringBuffer)) ||
            !pushArr(Type)(types, type)
        ) {
            exit(1); //Push param and check for malloc failures
        }
        getTok(); //Consume the param
    } while (curTok == tokComma && (getTok() || 1));
    return 1;
}

//function := type ident ( params? ) stmt
static Ast* parseFunction(Type type){
    //Assume the type has already been parsed and consumed
    if (curTok == tokIdent){
        char_t* name = toCstring(&stringBuffer);
        getTok(); //Consume identifier
        //Function call discovered
        if (curTok == tokLParen){
            NewAst(Function, func, type, name);
            initArr(vptr)(&func->paramNames, 0, NULL, &free);
            initArr(Type)(&func->paramTypes, 0, NULL, NULL); 
            if (curTok == tokRParen){
                goto finishedParsingSignature
            }
            else if (parseArgs(&func->paramNames, &func->paramTypes)){
                if (curTok == tokRParen){
                    finishedParsingSignature:
                    getTok(); //Consume right paren
                    Ast* stmt = parseStmt(); //Should be compound stmt
                    if (stmt) return stmt;
                }
            }
            disposeArr(vptr)(&func->paramNames);
            disposeArr(Type)(&func->paramTypes);
            disposeAst(func);
        }
        //TODO declarations as well
        free(name);
        return SyntaxError;
    }
}

Ast* topLevel(){
    Type type = parseType();
    if (type == typNone){
        //TODO handle non-functions
        return SyntaxError;
    }
    else{
        return parseFunction(type);
    }
}