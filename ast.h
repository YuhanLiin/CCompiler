#pragma once
#include "array.h"
#include "lexer.h"
#include "utils.h"
#include "type.h"

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
    astStmtDef,
    astFunction,
    astTopLevel
} Ast;

//Base structs are always first field. Contain ast label followed by attributes for semantic analysis
typedef struct {
    Ast label;
    Type type;
} ExprBase;
#define ExprBase(label) (ExprBase){label, typNone}

//Actual ast nodes and their allocators
typedef struct {
    ExprBase base; 
    double num;
} ExprDouble;
ExprDouble* newExprDouble(double num);

typedef struct {
    ExprBase base;
    long long num;
} ExprInt;
ExprInt* newExprInt(long long num);

typedef struct {
    ExprBase base; 
    char_t* str;
} ExprStr;
ExprStr* newExprStr(char_t* str);

typedef struct {
    ExprBase base; 
    char_t* name;
} ExprIdent;
ExprIdent* newExprIdent(char_t* name);

typedef struct {
    ExprBase base; 
    Token op; 
    ExprBase* left; 
    ExprBase* right;
} ExprBinop;
ExprBinop* newExprBinop(Token op, ExprBase* left, ExprBase* right);

typedef struct {
    ExprBase base; 
    char_t* name; 
    Array(vptr) args;
} ExprCall;
ExprCall* newExprCall(char_t* name);

typedef struct {Ast label;} StmtEmpty;
StmtEmpty* newStmtEmpty();

typedef struct {
    Ast label;
    ExprBase* expr;
} StmtReturn;
StmtReturn* newStmtReturn(ExprBase* expr);

typedef struct {
    Ast label;
    ExprBase* expr;
} StmtExpr;
StmtExpr* newStmtExpr(ExprBase* expr);

typedef struct {
    Ast label;
    Array(vptr) stmts;
} StmtBlock;
StmtBlock* newStmtBlock();

typedef struct {
    Ast label;  //Label can be either astStmtDef or astStmtDecl, depending on if it's variable definition or declaration
    Type type;
    char_t* name;   //Left null for unnamed params
} StmtVar;
StmtVar* newStmtVarDef(Type type, char_t* name);
StmtVar* newStmtVarDecl(Type type, char_t* name);

typedef struct {
    Ast label; 
    Type type; 
    char_t* name;
    Ast* stmt;  //Leave this null if there is no definition
    Array(vptr) params;  //List of StmtDef to represent parameters
} Function;
Function* newFunction(Type type, char_t* name);

typedef struct {
    Ast label;
    Array(vptr) globals;
} TopLevel;
TopLevel* newTopLevel();

//Delete ast based on type of ast
void disposeAst(void* ast);