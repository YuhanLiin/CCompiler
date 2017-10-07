#include "array.h"
#include "lexer.h"
#include "ast.h"
#include "utils.h"
#include "parser.h"
#include "symbols.h"
#include <stdlib.h>
#include <assert.h>

Token curTok;  //Lookahead token. Global lexer values correspond to this token
static Token getTok(){  //Updates lookahead token
    return curTok = lexToken();
}

void syntaxError(const char_t* expected){
    if (curTok == tokUnexpected){
        // Unexpected chars are not consumed, so we have to peek the next char to get it
        if (curChar == End){
            writeErr(lineTokenEndNumber, lineTokenEndPos, "expected %s, but found end of file.", expected);
        }
        else{
            writeErr(lineTokenEndNumber, lineTokenEndPos, "expected %s, but found '%c'.", expected, curChar);
        }
    }
    else{
        writeErr(lineNumber, linePos, "expected %s, but found %s.", expected, stringifyToken(curTok));
    }
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
        ExprBase* arg = parseExpr();
        if (arg == NULL) return 0; //If expression cant be parsed then syntax error
        if (!pushArr(vptr)(args, arg)) exit(1); //Push arg and check for malloc failures
    }while (curTok == tokComma && (getTok() || 1));   
    return 1;
}

//primeExpr := Number | String | ( expr ) | Ident ( args? )?
static ExprBase* parsePrimaryExpr(){
    switch (curTok){
        case tokNumInt: {
            NewAst(ExprInt, expr, intVal)
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumDouble: {
            NewAst(ExprDouble, expr, floatVal)
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokString: {
            NewAst(ExprStr, expr, toCstring(&stringBuffer))
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
            getTok(); //consume identifier
            //If bracket follows then its a function call
            if (curTok == tokLParen){  
                getTok(); //Consume left paren
                NewAst(ExprCall, call, name)
                //Initialize args array
                initArr(vptr)(&call->args, 0, NULL, &disposeAst);
                //If following brackets are not empty, attempt to parse 1 or more args.
                if (curTok == tokRParen){
                    getTok(); //Consume rb
                    return (ExprBase*)call;
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
                        return (ExprBase*)call;
                    }
                }
                disposeArr(vptr)(&call->args);
                free(call);
            }
            //Otherwise its a normal identifier
            else {
                NewAst(ExprIdent, ident, name)
                return (ExprBase*)ident;
            }
            free(name);
            return NULL;
        }
        default:
            syntaxError("expression");
            return NULL;
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
static ExprBase* parseBinopExpr(ExprBase* lhs, int minPrec){
    int prec;
    ExprBase* rhs = NULL;
    Token op;
    //Will first consume any higher/equal precedence binop. Afterwards, due to inner loop, this outer loop will only consume equal precedence binops
    while (operatorPrec(op = curTok) >= minPrec){
        getTok(); //Consume binop
        //Attempt to parse 1st atom of rhs expression
        if ((rhs = parsePrimaryExpr()) == NULL) return rhs;
        //While next binop is of higher precedence, accumulate expression into rhs
        while ((prec = operatorPrec(curTok)) > minPrec){
            ExprBase* newRhs = parseBinopExpr(rhs, prec);
            //Attempt to parse subsequent atoms at a precedece equal to current binop
            if (newRhs == NULL){
                disposeAst(rhs); //Responsible for delete rhs, which was created in this scope
                return newRhs;
            }
            rhs = newRhs;
        }
        //After rhs has been fully built, merge it with lhs and then continue
        NewAst(ExprBinop, newLhs, op, lhs, rhs)
        lhs = (ExprBase*)newLhs;
    }
    return lhs;
}

//expr := primeExpr exprBinop
ExprBase* parseExpr(){
    ExprBase* lhs = parsePrimaryExpr(); //Parse the 1st primary expression
    if (lhs == NULL) return lhs; 
    ExprBase* expr = parseBinopExpr(lhs, 1); //Parse all following binops
    if (expr == NULL){
        disposeAst(lhs);
    }
    return expr;
}

//Checks for semicolon at end of statement, throwing error if it's not found
static void checkSemicolon(){
    if (curTok == tokSemicolon){
        getTok(); //Consume semicolon
    }
    else{  
        syntaxError(stringifyToken(tokSemicolon));              
    }
}

Ast* parseStmt(){
    switch(curTok){
        case tokReturn: {
            getTok(); //Consume return
            ExprBase* expr = parseExpr();
            if (expr){
                NewAst(StmtReturn, stmt, expr)
                checkSemicolon();
                return (Ast*)stmt;
            }
            return NULL;
        }
        case tokSemicolon:
            getTok(); //Consume semicolon
            NewAst(StmtEmpty, stmt)
            return (Ast*)stmt;
        case tokLBrace:
            getTok(); //Consume left brace
            NewAst(StmtBlock, block)
            initArr(vptr)(&block->stmts, 0, NULL, &disposeAst);
            while (curTok != tokRBrace){
                Ast* stmt = parseStmt();
                if (stmt){
                    if (!pushArr(vptr)(&block->stmts, stmt)){
                        exit(1);
                    }
                }
                else{
                    disposeAst(block);
                    return NULL;
                }
            }
            getTok();
            return (Ast*)block;
            //TODO handle other statements

        //These are tokens that expressions can't start with, so they automatically trigger statement error
        case tokRBrace:
        case tokRParen:
        case tokUnexpected:
        case tokEof:
        case tokPlus:
        case tokMinus:
        case tokMulti:
        case tokDiv:
        case tokComma:
            syntaxError("statement");
            return NULL;
        //Checks for expression statements as last resort. The errors issued here will only be expression errors
        default: {
            ExprBase* expr = parseExpr();
            if (expr){
                NewAst(StmtExpr, stmt, expr);
                checkSemicolon();
                return (Ast*)stmt;
            }
            return NULL;
        }
    }      
}

static char parseParams(Array(Type) *types, Array(vptr) *names){
    //While comma exists, consume it (the getTok() call) and keep parsing identifiers
    do {    
        Type type = parseType();
        //Each param must consist of a type and a name
        if (type == typNone){ 
            syntaxError("type name");
            return 0;
        }
        if (curTok != tokIdent){
            syntaxError(stringifyToken(tokIdent));
            return 0;
        }
        if (!pushArr(vptr)(names, toCstring(&stringBuffer)) ||
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
            getTok(); //Consume left paren
            NewAst(Function, func, type, name)
            initArr(vptr)(&func->paramNames, 0, NULL, &free);
            initArr(Type)(&func->paramTypes, 0, NULL, NULL); 
            if (curTok == tokRParen){
                getTok(); //Consume right paren
                goto finishedParsingParams;
            }
            // Parse parameters. This will report errors so no need to do it here
            else if (parseParams(&func->paramTypes, &func->paramNames)){
                //Right paren error recovery
                if (curTok == tokRParen){
                    getTok(); //Consume right paren
                }
                else{
                    syntaxError(stringifyToken(tokRParen));
                }
                finishedParsingParams:
                //Parse either ; for declaration or a statement for definition
                if (curTok == tokSemicolon){
                    getTok();
                    return (Ast*)func;
                }
                else if (func->stmt = parseStmt()){
                    return (Ast*)func;
                }
                //parseStmt reports errors, so no need for it here
            }
            disposeArr(vptr)(&func->paramNames);
            disposeArr(Type)(&func->paramTypes);
            free(func);
        }
        //TODO declarations as well
        free(name);
    }
    else{
        syntaxError(stringifyToken(tokIdent));
    }
    return NULL;
}

Ast* parseTopLevel(){
    Type type = parseType();
    if (type == typNone){
        //TODO handle non-functions
        return NULL;
    }
    else{
        return parseFunction(type);
    }
}