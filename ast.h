#pragma once
#include "array.h"
#include "lexer.h"
#include "utils.h"
#include "symbols.h"

//Acts as a label for ast node type and also base class
//All nodes must have this as 1st field so that the labels can be checked before casting into correct node type
typedef enum {
    astExprDouble,
    astExprInt,
    astExprStr,
    astExprIdent,
    astExprCall,
    astExprBinop,
    astStmtEmpty,
    astStmtReturn,
    astStmtExpr,
    astStmtBlock,
    astStmtDecl,
    astFunction
} Ast;

//Base structs are always first field. Contain ast label followed by attributes for semantic analysis
typedef struct {
    Ast label;
    Type type;
} ExprBase;

//Below macros will function like c++ constructors for ast nodes
typedef struct {
    ExprBase base; 
    double num;
} ExprDouble;
#define ExprDouble(num) (ExprDouble){(ExprBase){astExprDouble, typNone}, num}

typedef struct {
    ExprBase base;
    long long num;
} ExprInt;
#define ExprInt(num) (ExprInt){(ExprBase){astExprInt, typNone}, num}

typedef struct {
    ExprBase base; 
    char_t* str;
} ExprStr;
#define ExprStr(str) (ExprStr){(ExprBase){astExprStr, typNone}, str}

typedef struct {
    ExprBase base; 
    char_t* name;
} ExprIdent;
#define ExprIdent(name) (ExprIdent){(ExprBase){astExprIdent, typNone}, name}

typedef struct {
    ExprBase base; 
    Token op; 
    ExprBase* left; 
    ExprBase* right;
} ExprBinop;
#define ExprBinop(op, left, right) (ExprBinop){(ExprBase){astExprBinop, typNone}, op, left, right}

typedef struct {
    ExprBase base; 
    char_t* name; 
    Array(vptr) args;
} ExprCall;
#define ExprCall(id) (ExprCall){(ExprBase){astExprCall, typNone}, id}

typedef struct {Ast label;} StmtEmpty;
#define StmtEmpty(expr) (StmtEmpty){astStmtEmpty}

typedef struct {
    Ast label;
    ExprBase* expr;
} StmtReturn;
#define StmtReturn(expr) (StmtReturn){astStmtReturn, expr}

typedef struct {
    Ast label;
    ExprBase* expr;
} StmtExpr;
#define StmtExpr(expr) (StmtExpr){astStmtExpr, expr}

typedef struct {
    Ast label;
    Array(vptr) stmts;
} StmtBlock;
#define StmtBlock() (StmtBlock){astStmtBlock}

typedef struct {
    Ast label;
    Type type;
    char_t* name;
} StmtDecl;
#define StmtDecl(type, name) (StmtDecl){astStmtDecl, type, name}

typedef struct {
    Ast label; 
    Type type; 
    char_t* name;
    Ast* stmt;  //Leave this null if there is no definition
    Array(Type) paramTypes; 
    Array(vptr) paramNames; 
} Function;
#define Function(type, name) (Function){astFunction, type, name, NULL}

#define NewAst(T, ptrname, ...) New(T, ptrname, 1) *ptrname = T(__VA_ARGS__);

//Delete ast based on type of ast
void disposeAst(void* ast);