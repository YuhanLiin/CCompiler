#include "lexer/lexer.h"
#include "ast/ast.h"
#include "utils.h"
#include "io/error.h"
#include "semantics/semantics.h"

extern Token curTok;
char checkSyntax();
Token getTok();
void syntaxError(const char_t* expected);
ExprBase* parseExpr();
