#pragma once
#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/type.h"
#include <stdint.h>

//Acts as a label for ast node type and also base class
//All nodes must have this as 1st field so that the labels can be checked before casting into correct node type
typedef enum {
    astExprFloat,
    astExprDouble,
    astExprLong,
    astExprInt,
    astExprStr,
    astExprIdent,
    astExprCall,
    astExprUnop,
    astExprBinop,
    astStmtEmpty,
    astStmtBreak,
    astStmtContinue,
    astStmtReturn,
    astStmtExpr,
    astStmtBlock,
    astStmtDecl,
    astStmtDef,
    astStmtWhile,
    astStmtDoWhile,
    astFunction,
    astTopLevel
} AstLabel;

//Base structs are always first field. Contain ast label followed by attributes for semantic analysis
typedef struct {
    AstLabel label;
    size_t lineNumber;
    size_t linePos;
} Ast;

typedef struct {
    Ast ast;
    Type type;
} ExprBase;

//Actual ast nodes and their allocators. Output should be a valid AST node for semantic validation

//Statements that hold no actual data just use the base Ast struct instead of some other ast struct
Ast* newStmtEmpty(size_t label, size_t lineNumber);
Ast* newStmtBreak(size_t label, size_t lineNumber);
Ast* newStmtContinue(size_t label, size_t lineNumber);

typedef struct {
    ExprBase base; 
    double num;
} ExprDouble;
ExprDouble* newExprDouble(size_t lineNumber, size_t linePos, double num);

typedef struct {
    ExprBase base;
    float num;
} ExprFloat;
ExprFloat* newExprFloat(size_t lineNumber, size_t linePos, float num);

typedef struct {
    ExprBase base;
    uint32_t num;
} ExprInt;
ExprInt* newExprInt(size_t lineNumber, size_t linePos, uint32_t num);

typedef struct {
    ExprBase base;
    uint64_t num;
} ExprLong;
ExprLong* newExprLong(size_t lineNumber, size_t linePos, uint64_t num);

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
    ExprBase* operand;
    char leftside;
} ExprUnop;
ExprUnop* newExprUnop(size_t lineNumber, size_t linePos, Token op, ExprBase* operand, char leftside);

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
    ExprBase* rhs;  //Left null for declarations or if no assignment is made. Can only have a value if its a definition
} StmtVar;
StmtVar* newStmtVarDef(size_t lineNumber, size_t linePos, Type type, char_t* name);
StmtVar* newStmtVarDecl(size_t lineNumber, size_t linePos, Type type, char_t* name);

//Used for both while and do while loops
typedef struct {
    Ast ast;
    ExprBase* condition;
    Ast* stmt;
} StmtWhileLoop;
StmtWhileLoop* newStmtWhile(size_t lineNumber, size_t linePos, ExprBase* condition, Ast* stmt);
StmtWhileLoop* newStmtDoWhile(size_t lineNumber, size_t linePos, ExprBase* condition, Ast* stmt);

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
char hasRetExpr(const StmtReturn* ret);
