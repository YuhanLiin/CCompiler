#pragma once
#include "array.h"
#include "lexer.h"
#include "utils.h"
#include "semantic.h"

//Acts as a label for ast node type and also base class
//All nodes must have this as 1st field so that the labels can be checked before casting into correct node type
typedef enum AstEnum {
    astExprDouble,
    astExprInt,
    astExprStr,
    astExprIdent,
    astExprCall,
    astExprBinop,
    astStmtReturn,
    astFunction
} Ast;

//Below macros will function like c++ constructors for ast nodes
typedef struct {
    Ast label; 
    double num;
} ExprDouble;
#define ExprDouble(num) (ExprDouble){astExprDouble, num}

typedef struct {
    Ast label;
    long long num;
} ExprInt;
#define ExprInt(num) (ExprInt){astExprInt, num}

typedef struct {
    Ast label; 
    char_t* str;
} ExprStr;
#define ExprStr(str) (ExprStr){astExprStr, str}

typedef struct {
    Ast label; 
    char_t* name;
} ExprIdent;
#define ExprIdent(name) (ExprIdent){astExprIdent, name}

typedef struct {
    Ast label; 
    Token op; 
    Ast* left; 
    Ast* right;
} ExprBinop;
#define ExprBinop(op, left, right) (ExprBinop){astExprBinop, op, left, right}

typedef struct {
    Ast label; 
    char_t* name; 
    Array(vptr) args;
} ExprCall;
#define ExprCall(id) (ExprCall){astExprCall, id}

typedef struct {
    Ast label;
    Ast* expr;
} StmtReturn;
#define StmtReturn(expr) (StmtReturn){astStmtReturn, expr}

typedef struct {
    Ast label; 
    Type type; 
    char_t* name; 
    Array(Type) paramTypes; 
    Array(vptr) paramNames; 
    Ast* stmt;  //Leave this null if there is no definition
} Function;
#define Function(type, name) (Function){astFunction, type, name}    //Bad to copy so many fields, so limited constructor

//Delete ast based on type of ast
void disposeAst(void* ast);