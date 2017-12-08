#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "io/error.h"
#include "ast/type.h"
#include <string.h>
#include <assert.h>

static Type returnType = typNone;
static char correct = 1;
static size_t breakDepth = 0;
static size_t continueDepth = 0;

void initSemantics(){
    returnType = typNone;
    correct = 1;
    breakDepth = 0;
    continueDepth = 0;
}
char checkSemantics(){
    return correct;
}

#define VOID_ERROR_MSG "cannot use a void-returning function call as an expression."

#define semanticError(ast, ...) do {correct = 0; writeError((ast).lineNumber, (ast).linePos, __VA_ARGS__);} while(0)

static char validateVarDefine(const StmtVar* var){
    const Ast* sym = findSymbolCurScope(var->name);
    // If variable name exists and is not an externed variable, the new var can't be defined
    return sym == NULL || sym->label == astStmtDecl;
}

static char validateFuncDefine(const Function* func){
    const Ast* sym = findSymbolCurScope(func->name);
    //If name doesn't exist, then we can define new func
    if (sym == NULL){
        return 1;
    }
    //If the symbol is a variable or func definition, we cannot define new func
    const Function* prev = (const Function*)sym;
    if (sym->label != astFunction || !isFuncDecl(prev)){
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

//All expression validators must set expression type or leave them as typNone if validation fails completely
ExprInt* verifyExprInt(ExprInt* expint){
    expint->base.type = typInt32;
    return expint;
}
ExprInt* verifyExprUnsignedInt(ExprInt* expint){
    expint->base.type = typUInt32;
    return expint;
}

ExprLong* verifyExprLong(ExprLong* expint){
    expint->base.type = typInt64;
    return expint;
}
ExprLong* verifyExprUnsignedLong(ExprLong* expint){
    expint->base.type = typUInt64;
    return expint;
}

ExprDouble* verifyExprDouble(ExprDouble* expdb){
    expdb->base.type = typFloat64;
    return expdb;
}
ExprFloat* verifyExprFloat(ExprFloat* expdb){
    expdb->base.type = typFloat32;
    return expdb;
}

static char isLvalue(ExprBase* expr){
    switch(expr->ast.label){
        case astExprIdent:
            return 1;
    }
    return 0;
}

ExprUnop* verifyExprUnop(ExprUnop* unop){
    Type operandType = unop->operand->type;
    switch(unop->op){
        case tokDec:
        case tokInc:
            if (!isLvalue(unop->operand)){
                semanticError(unop->base.ast, "lvalue required as operand of %s.", stringifyToken(unop->op));
                break;
            }
        case tokMinus:
            unop->base.type = operandType;
            break;
        default:
            assert(0 && "Not an unary token operator");
    }
    return unop;
}

//Assume that both expression types are not void nor none
static void verifyArithBinop(ExprBinop* binop){
    Type promoted = arithTypePromotion(binop->left->type, binop->right->type);
    if (promoted != typNone){
        binop->base.type = promoted;
    }
    else{
        semanticError(
            binop->base.ast, "invalid types %s and %s for %s operator.", 
            stringifyType(binop->left->type), stringifyType(binop->right->type), stringifyToken(binop->op)
        );
    }
}
static void verifyAssignBinop(ExprBinop* binop){
    if (!isLvalue(binop->left)){
        semanticError(binop->left->ast, "lvalue required on left of assignment.");
        return;
    }
    if (binop->op == tokAssign){
        if (!checkTypeConvert(binop->right->type, binop->left->type)){
            semanticError(
                binop->right->ast, "no way to assign from %s to %s.",
                stringifyType(binop->right->type), stringifyType(binop->left->type)
            );
        }
    }
    else{
        if (arithTypePromotion(binop->right->type, binop->left->type) == typNone){
            semanticError(
                binop->base.ast, "invalid types %s and %s for %s operator.", 
                stringifyType(binop->left->type), stringifyType(binop->right->type), stringifyToken(binop->op)
            );
        }
    }
    binop->base.type = binop->left->type;
}

ExprBinop* verifyExprBinop(ExprBinop* binop){
    if (binop->left->type == typVoid || binop->right->type == typVoid){
        if (binop->left->type == typVoid){
            semanticError(binop->left->ast, VOID_ERROR_MSG);
        }
        if (binop->right->type == typVoid){
            semanticError(binop->right->ast, VOID_ERROR_MSG);
        }
    }
    //Perform type verifcation only if both expression types have been verified and are not void
    else if (binop->left->type != typNone && binop->right->type != typNone){
        switch(binop->op){
            case tokPlus:
            case tokMinus:
            case tokDiv:
            case tokMulti:
                verifyArithBinop(binop);
                break;
            case tokAssign:
            case tokPlusAssign:
            case tokMinusAssign:
            case tokDivAssign:
            case tokMultiAssign:
                verifyAssignBinop(binop);
                break;
            default:
                assert(0 && "Unhandled binop.");
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
        semanticError(ident->base.ast, "attempting to reference undeclared variable '%s'.", ident->name);
    }
    return ident;
}

static void verifyArgs(ExprCall* call, const Function* func){
    const Array(vptr)* args = &call->args;
    const Array(vptr)* params = &func->params;
    if (args->size == params->size){
        for (size_t i=0; i<args->size; i++){
            ExprBase* arg = (ExprBase*)args->elem[i];
            StmtVar* param = (StmtVar*)params->elem[i];
            if (arg->type == typVoid){
                semanticError(arg->ast, VOID_ERROR_MSG);
            }
            //Typecheck only valid args
            else if (arg->type != typNone){
                if (!checkTypeConvert(arg->type, param->type)){
                    semanticError(
                        arg->ast,
                        "no way to convert argument type '%s' to parameter type '%s'.",
                        stringifyType(arg->type), stringifyType(param->type)
                    );
                }  
            }
        }
        return;
    }
    semanticError(call->base.ast, "wrong number of arguments for calling function '%s'.", func->name);
}

ExprCall* verifyExprCall(ExprCall* call){
    const Function* func = findFunc(call->name);
    if (func){
        verifyArgs(call, func);
        call->base.type = func->type;
    }
    else {
        semanticError(call->base.ast, "attempting to call undeclared function '%s'.", call->name);
    }
    return call;
}

void preverifyBlockStmt(){
    toNewScope();
}

StmtBlock* verifyBlockStmt(StmtBlock* blk){
    blk->scopeId = curScope;
    return blk;
}

void postVerifyBlockStmt(){
    toPrevScope();
}

StmtReturn* verifyStmtReturn(StmtReturn* ret){
    if (returnType == typNone){
        // Global scope, cant return
        semanticError(ret->ast, "returning from global scope.");
    }
    else if (hasRetExpr(ret)){
        if (returnType == typVoid){
            semanticError(ret->ast, "cannot return a value from a void function.");
        }
        else if (ret->expr->type == typVoid){
            semanticError(ret->expr->ast, VOID_ERROR_MSG);
        }
        //Check if expression type was findable before matching it to return type. Prevents cascading errors
        else if (ret->expr->type != typNone){
            if (!checkTypeConvert(ret->expr->type, returnType)){
                semanticError(
                    ret->expr->ast,
                    "no way to convert value type '%s' to return type '%s'.",
                    stringifyType(ret->expr->type), stringifyType(returnType)
                );
            }
        }
    }
    return ret;
}

StmtVar* verifyStmtVar(StmtVar* var){
    if (validateVarDefine(var)){
        if (var->rhs && var->rhs != typNone){
            if (var->rhs->type == typVoid){
                semanticError(var->rhs->ast, VOID_ERROR_MSG);
            }
            else if (!checkTypeConvert(var->rhs->type, var->type)){
                semanticError(var->rhs->ast, "no way to assign from %s to %s.", stringifyType(var->rhs->type), stringifyType(var->type));
            }
        }
    }
    else{
        semanticError(var->ast, "variable '%s' has already been defined.", var->name);
    }
    insertVar(var->name, var);
    return var;
}

void preverifyLoop(){
    breakDepth++;
    continueDepth++;
}
void postVerifyLoop(){
    assert(breakDepth > 0 && continueDepth > 0 && "Probably forgot matching preverifyLoop");
    breakDepth--;
    continueDepth--;
}

Ast* verifyStmtBreak(Ast* stmt){
    if (breakDepth == 0){
        semanticError(*stmt, "break statement placed outside of loop or switch block.");
    }
    return stmt;
}
Ast* verifyStmtContinue(Ast* stmt){
    if (continueDepth == 0){
        semanticError(*stmt, "continue statement placed outside of loop.");
    }
    return stmt;
}

static void verifyParamTypes(Array(vptr)* params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        if (param->type == typVoid){
            semanticError(param->ast, "function parameters cannot have type void.");
        }
    }
}

static void verifyAndSetParams(Array(vptr)* params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        if (param->name == NULL){
            semanticError(param->ast, "nameless parameter in function definition.");
            return;
        }
        if (!validateVarDefine(param)){
            semanticError(param->ast, "parameter '%s' has already been defined.", param->name);
            return;
        }
        insertVar(param->name, param);
    }
}

//To be called after parsing params and before parsing function body
void verifyFunctionSignature(Function* func, char isDecl){
    if (!validateFuncDefine(func)){
        semanticError(func->ast, "function name '%s' has already been defined.", func->name);
    }
    else if (!validateFuncPrevDecl(func)){
        semanticError(func->ast, "definition of '%s' does not match its previous declaration.", func->name);
    }
    else{
        insertFunc(func->name, func);
    }
    verifyParamTypes(&func->params);
    if (!isDecl){
        toNewScope();
        func->scopeId = curScope;
        returnType = func->type;
        verifyAndSetParams(&func->params);
    }
}

//Call only after definition
void verifyFunctionBody(){
    returnType = typNone;
    toPrevScope();
}