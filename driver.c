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

static void changeExtension(char_t* dest, const char_t* src, const char_t* ext){
    strcpy(dest, src);
    char_t* pDot = strchr(dest, '.');
    if (pDot == NULL){
        pDot = dest + strlen(dest);
    }
    else{
        pDot++;
    }
    sprintf(pDot, ext);
}

int driver(int argc, char_t const *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Error: Need an input file.\n");
        return 2;
    }
    const char_t* infilename = argv[1];
    char_t outfilename[100];
    changeExtension(outfilename, infilename, "s");
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
        char_t exefilename[100];
        changeExtension(exefilename, infilename, "exe");
        char_t gccCommand[100];
        sprintf(gccCommand, "gcc %s -o %s", outfilename, exefilename);
        return system(gccCommand);
    }
    return code;
}