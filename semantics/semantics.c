#include "lexer/lexer.h"
#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "io/error.h"
#include "ast/type.h"
#include <string.h>

static Type returnType = typNone;
static char correct = 1;

void initSemantics(){
    returnType = typNone;
    correct = 1;
}
char checkSemantics(){
    return correct;
}

#define semanticError(...) do {correct = 0; writeError(lineNumber, linePos, __VA_ARGS__);} while(0)

static char validateVarDefine(const StmtVar* var){
    const Ast* sym = findSymbolCurScope(var->name);
    // If variable name exists and is not an externed variable, the new var can't be defined
    return sym == NULL || *sym == astStmtDecl;
}

static char validateFuncDefine(const Function* func){
    const Ast* sym = findSymbolCurScope(func->name);
    //If name doesn't exist, then we can define new func
    if (sym == NULL){
        return 1;
    }
    //If the symbol is a variable or func definition, we cannot define new func
    const Function* prev = (const Function*)sym;
    if (*sym != astFunction || !isFuncDecl(prev)){
        return 0;
    }
    return 1;
}

//Assume previous occurence of function exists and is a declaration
static char validateFuncPrevDecl(const Function* func){
    const Function* prev = findFunc(func->name);
    if (prev == NULL) return 1;
    // Even if previous occurrence was a declaration, still need to check whether the signatures are equal
    if (func->type == prev->type && func->params.size == prev->params.size){
        for (size_t i=0; i<func->params.size; i++){
            if (((StmtVar*)func->params.elem[i])->type != ((StmtVar*)func->params.elem[i])->type){
                return 0;
            }
        }
        return 1;
    }
    return 0; 
}

// static void verifyArgs(const ExprCall* call, const Function* func){
//     const Array(vptr)* args = &call->args;
//     const Array(vptr)* params = &func->params;
//     if (args->size == params->size){
//         for (size_t i=0; i<args->size; i++){
//             ExprBase* arg = (ExprBase*)args->elem[i];
//             StmtVar* param = (StmtVar*)params->elem[i];
//             verifyExpr(arg);
//             if (arg->type != typNone){
//                 if (arg->type != param->type){
//                     semanticError();
//                 }  
//             }
//         }
//         return;
//     }
//     semanticError();
// }

static void verifyArgs(const ExprCall* call, const Function* func){
    const Array(vptr)* args = &call->args;
    const Array(vptr)* params = &func->params;
    if (args->size == params->size){
        for (size_t i=0; i<args->size; i++){
            ExprBase* arg = (ExprBase*)args->elem[i];
            StmtVar* param = (StmtVar*)params->elem[i];
            //Typecheck only valid args
            if (arg->type != typNone){
                if (arg->type != param->type){
                    semanticError(
                        "no way to convert argument type '%s' to parameter type '%s'.",
                        stringifyType(arg->type), stringifyType(param->type)
                    );
                }  
            }
        }
        return;
    }
    semanticError("wrong number of arguments for calling function '%s'.", func->name);
}

ExprInt* verifyExprInt(ExprInt* expint){
    expint->base.type = typInt32;
    return expint;
}

ExprDouble* verifyExprDouble(ExprDouble* expdb){
    expdb->base.type = typFloat32;
    return expdb;
}

ExprBinop* verifyExprBinop(ExprBinop* binop){
    //Perform type verifcation only if both expression types have been verified
    if (binop->left->type != typNone && binop->right->type != typNone){
        if (binop->left->type == binop->right->type){
            binop->base.type = binop->left->type;
        }
        else{
            semanticError(
                "invalid types %s and %s for %s operator.", 
                stringifyType(binop->left->type), stringifyType(binop->right->type), stringifyToken(binop->op)
            );
        }
    }
    return binop;
}

ExprIdent* verifyExprIdent(ExprIdent* ident){
    const StmtVar* value = findVar(ident->name);
    if (value){
        ident->base.type = value->type;
    }
    else {
        semanticError("attempting to reference undeclared variable '%s'.", ident->name);
    }
    return ident;
}

ExprCall* verifyExprCall(ExprCall* call){
    const Function* func = findFunc(call->name);
    if (func){
        verifyArgs(call, func);
        call->base.type = func->type;
    }
    else {
        semanticError("attempting to call undeclared function '%s'.", call->name);
    }
    return call;
}

//Fills in the type attributes. Leave them as typNone if there's no way to find the type
// static void verifyExpr(ExprBase* expr){
//     switch(expr->label){
//         case astExprInt:
//             expr->type = typInt32;
//             return;
//         case astExprDouble:
//             expr->type = typFloat32;
//             return;
//         case astExprBinop: {
//             ExprBinop* binop = (ExprBinop*)expr;
//             verifyExpr(binop->left);
//             verifyExpr(binop->right);
//             if (binop->left->type != typNone && binop->right->type != typNone){
//                 if (binop->left->type == binop->right->type){
//                     expr->type = binop->left->type;
//                     return;
//                 }
//                 semanticError();
//             }
//             return;
//         }
//         case astExprIdent: {
//             const StmtVar* value = findVar(((ExprIdent*)expr)->name);
//             if (value){
//                 expr->type = value->type;
//                 return;
//             }
//             semanticError();
//             return;
//         }
//         case astExprCall: {
//             ExprCall* call = (ExprCall*)expr;
//             const Function* func = findFunc(call->name);
//             if (func){
//                 expr->type = func->type;
//                 verifyArgs(call, func);
//                 return;
//             }
//             semanticError();
//             return;
//         }
//         default:
//             //TODO support more expr ast types
//             assert(0 && "Invalid AST for expr");
//     }
// }

//static void verifyStmt(Ast* ast);

// static void verifyBlockStmt(StmtBlock* blk){
//     for (size_t i=0; i<blk->stmts.size; i++){
//         verifyStmt(blk->stmts.elem[i]);
//     }
// }

void preverifyBlockStmt(){
    toNewScope();
}

StmtBlock* verifyBlockStmt(StmtBlock* blk){
    verifyBlockStmt(blk);
    toPrevScope();
    return blk;
}

StmtReturn* verifyStmtReturn(StmtReturn* ret){
    //TODO auto-casts
    if (returnType == typNone){
        // Global scope, cant return
        semanticError("returning from global scope.");
    }
    //Check if expression type was findable before matching it to return type. Prevents cascading errors
    else if (ret->expr->type != typNone){
        if (ret->expr->type != returnType){
            semanticError(
                "no way to convert value type '%s' to return type '%s'.",
                stringifyType(ret->expr->type), stringifyType(returnType)
            );
        }
    }
    return ret;
}

// static void verifyStmt(Ast* ast){
//     switch(*ast){
//         case astStmtReturn:{
//             //TODO auto-casts
//             StmtReturn* ret = (StmtReturn*)ast;
//             verifyExpr(ret->expr);
//             if (returnType == typNone){
//                 // Global scope, cant return
//                 semanticError();
//                 return;
//             }
//             if (ret->expr->type != typNone){
//                 if (ret->expr->type != returnType){
//                     semanticError();
//                 }
//             }
//             return;
//         }
//         case astStmtEmpty:
//             return;
//         case astStmtExpr:
//             verifyExpr(((StmtExpr*)ast)->expr);
//             return;
//         case astStmtBlock: {
//             toNewScope();
//             ((StmtBlock*)ast)->scopeId = curScope;
//             verifyBlockStmt((StmtBlock*)ast);
//             toPrevScope();
//             return;
//         }
//         default:
//             //TODO support more ast types
//             assert(0 && "Invalid AST for stmt");
//     }
// }

static void verifyAndSetParams(Array(vptr)* params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        if (param->name == NULL){
            semanticError("nameless parameter in function definition.");
        }
        else if (!validateVarDefine(param)){
            semanticError("parameter '%s' has already been defined", param->name);
        }
        else{
            insertVar(param->name, param);
        }
    }
}

//To be called after parsing params and before parsing function body
void preverifyFunction(Function* func, char isDecl){
    if (!validateFuncDefine(func)){
        semanticError("function name '%s' has already been defined.", func->name);
    }
    else if (!validateFuncPrevDecl(func)){
        semanticError("definition of '%s' does not match its previous declaration", func->name);
    }
    else{
        insertFunc(func->name, func);
    }
    if (!isDecl){
        toNewScope();
        returnType = func->type;
        verifyAndSetParams(&func->params);
    }
}

//Call only after definition
void verifyFunctionDefinition(){
    returnType = typNone;
    toPrevScope();
}

// static void verifyGlobal(Ast* ast){
//     switch(*ast){
//         case astFunction: {
//             Function *func = (Function*)ast;
//             if (validateFuncDefine(func)){
//                 semanticError();
//             }
//             insertFunc(func->name, func);
//             if (func->stmt == NULL){
//                 //Declaration only, so no more content to verify
//                 return;
//             }
//             toNewScope();
//             func->scopeId = curScope;
//             verifyAndSetParams(&func->params);
//             returnType = func->type;
//             if (*func->stmt == astStmtBlock){
//                 verifyBlockStmt((StmtBlock*)func->stmt);
//                 ((StmtBlock*)func->stmt)->scopeId = curScope;
//             }
//             else {
//                 verifyStmt(func->stmt);
//             }
//             returnType = typNone;
//             toPrevScope();
//             return;
//         }
//         default:
//             //TODO support more top level ast types
//             assert(0 && "Invalid AST for top level");
//     }
// }

// void verifyTopLevel(TopLevel* top){
//     assert(top != NULL && "Can't perform semantic analysis on bad AST");
//     for (size_t i=0; i<top->globals.size; i++){
//         verifyGlobal(top->globals.elem[i]);
//     }
// }