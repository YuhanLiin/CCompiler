#include "utils.h"
#include "io/file.h"
#include "io/error.h"
#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/semantics.h"
#include "codegen/codegen.h"
#include "semantics/symtable.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int driver(int argc, char_t const *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Error: Need an input and an output file.\n");
        return 2;
    }
    const char_t* infilename = argv[1];
    const char_t* outfilename = argv[2];
    if (!strcmp(infilename, outfilename)){
        fprintf(stderr, "Error: Input and output files can't be the same.\n");
        return 2;
    }
    openFiles(infilename, outfilename);

    initLexer();
    initParser();
    initSymbolTable();
    int code = 0;
    TopLevel* ast = parseTopLevel();
    if (ast != NULL){
        if (checkSemantics() && checkSyntax()){
            cmplTopLevel(ast);
        }
        else{
            code = 3;   
        }
        disposeAst(ast);
    }
    else {
        code = 3;
    }
    disposeSymbolTable();
    disposeLexer();
    closeFiles(infilename, outfilename);

    if (code == 0){
        char_t gccCommand[1000];
        sprintf(gccCommand, "gcc %s", outfilename);
        return system(gccCommand);
    }
    return code;
}