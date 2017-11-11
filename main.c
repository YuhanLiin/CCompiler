#include "utils.h"
#include "io/file.h"
#include "io/error.h"
#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "codegen/codegen.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Error: No C file name.\n");
        return 2;
    }
    const char_t* infilename = argv[1];
    const char_t* outfilename = argv[2];
    openFiles(infilename, outfilename);

    initLexer();
    initParser();
    int code = 0;
    TopLevel* ast = parseTopLevel();
    if (ast != NULL){
        initScopes();
        initSymbolTable();
        initSemantics();
        verifyTopLevel(ast);
        if (checkSemantics()){
            cmplTopLevel(ast);
        }
        else{
            code = 3;   
        }
        disposeScopes();
        disposeSymbolTable();
        disposeAst(ast);
    }
    else {
        code = 3;
        printf("Semantic error placeholder\n");
    }
    disposeLexer();
    closeFiles(infilename, outfilename);
    return code;
}