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
    expdb->base.type = typFloat32;
    return expdb;
}
ExprFloat* verifyExprFloat(ExprFloat* expdb){
    expdb->base.type = typFloat64;
    return expdb;
}

ExprBinop* verifyExprBinop(ExprBinop* binop){
    //Perform type verifcation only if both expression types have been verified
    if (binop->left->type != typNone && binop->right->type != typNone){
        Type promoted = arithTypePromotion(binop->left->type, binop->right->type);
        if (promoted != typNone){
            binop->base.type = binop->left->type;
        }
        else{
            semanticError(
                binop->base.ast, "invalid types %s and %s for %s operator.", 
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
            //Typecheck only valid args
            if (arg->type != typNone){
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
    toPrevScope();
    return blk;
}

StmtReturn* verifyStmtReturn(StmtReturn* ret){
    //TODO auto-casts
    if (returnType == typNone){
        // Global scope, cant return
        semanticError(ret->ast, "returning from global scope.");
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
    return ret;
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