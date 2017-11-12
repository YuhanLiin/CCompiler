#pragma once
#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/type.h"

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
} AstLabel;

typedef struct {
    AstLabel label;
    size_t lineNumber;
    size_t linePos;
} Ast;

//Base structs are always first field. Contain ast label followed by attributes for semantic analysis
typedef struct {
    Ast ast;
    Type type;
} ExprBase;

//Actual ast nodes and their allocators
typedef struct {
    ExprBase base; 
    double num;
} ExprDouble;
ExprDouble* newExprDouble(size_t lineNumber, size_t linePos, double num);
ExprDouble* newExprFloat(size_t lineNumber, size_t linePos, float num);

typedef struct {
    ExprBase base;
    unsigned long long num;
} ExprInt;
ExprInt* newExprInt(size_t lineNumber, size_t linePos, unsigned int num);
ExprInt* newExprLong(size_t lineNumber, size_t linePos, unsigned long long num);

typedef struct {
    ExprBase base; 
    char_t* str;
} ExprStr;
ExprStr* newExprStr(size_t lineNumber, size_t linePos, char_t* str);

typedef struct {
    ExprBase base; 
    char_t* name;
} ExprIdent;
ExprIdent* newExprIdent(size_t lineNumber, size_t linePos, char_t* name);

typedef struct {
    ExprBase base; 
    Token op; 
    ExprBase* left; 
    ExprBase* right;
} ExprBinop;
ExprBinop* newExprBinop(size_t lineNumber, size_t linePos, Token op, ExprBase* left, ExprBase* right);

typedef struct {
    ExprBase base; 
    char_t* name; 
    Array(vptr) args;
} ExprCall;
ExprCall* newExprCall(size_t lineNumber, size_t linePos, char_t* name);

typedef struct {Ast ast;} StmtEmpty;
StmtEmpty* newStmtEmpty(size_t label, size_t lineNumber);

typedef struct {
    Ast ast;
    ExprBase* expr;  //NULL means no return value
} StmtReturn;
StmtReturn* newStmtReturn(size_t lineNumber, size_t linePos);

typedef struct {
    Ast ast;
    ExprBase* expr;
} StmtExpr;
StmtExpr* newStmtExpr(size_t lineNumber, size_t linePos, ExprBase* expr);

typedef struct {
    Ast ast;
    size_t scopeId;
    Array(vptr) stmts;
} StmtBlock;
StmtBlock* newStmtBlock(size_t label, size_t lineNumber);

typedef struct {
    Ast ast;  //Label can be either astStmtDef or astStmtDecl, depending on if it's variable definition or declaration
    Type type;
    char_t* name;   //Left null for unnamed params
} StmtVar;
StmtVar* newStmtVarDef(size_t lineNumber, size_t linePos, Type type, char_t* name);
StmtVar* newStmtVarDecl(size_t lineNumber, size_t linePos, Type type, char_t* name);

typedef struct {
    Ast ast;
    size_t scopeId;
    Type type; 
    char_t* name;
    Ast* stmt;  //Leave this null if there is no definition
    Array(vptr) params;  //List of StmtDef to represent parameters
} Function;
Function* newFunction(size_t lineNumber, size_t linePos, Type type, char_t* name);

typedef struct {
    Ast ast;
    Array(vptr) globals;
} TopLevel;
TopLevel* newTopLevel();

//Delete ast based on type of ast
void disposeAst(void* ast);

char isVarDecl(const StmtVar* var);
char isFuncDecl(const Function* func);