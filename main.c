#include "utils.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

FILE* infile;
FILE* outfile;

void emitLine(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(outfile, format, args);
    fprintf(outfile, "\n");
    va_end(args);
}

char_t consumeNext(){
    int c = fgetc(infile);
    if (c == EOF){
        return End;
    }
    return c;
}

void safeClose(FILE* file, const char* filename){
    if (fclose(file) != 0){
        fprintf(stderr, "Warning: File %s did not close successfully\n", filename);
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 3){
        printf("Error: No C file name.\n");
        return 2;
    }
    const char* infilename = argv[1];
    const char* outfilename = argv[2];
    infile = fopen(infilename, "r");
    if (infile == NULL){
        fprintf(stderr, "Error: Input file %s couldn't be opened.\n", infilename);
        return 2;
    }
    outfile = fopen(outfilename, "w");
    if (outfile == NULL){
        safeClose(infile, infilename);
        fprintf(stderr, "Error: Output file %s couldn't be opened.\n", outfilename);
        return 2;
    }

    initLexer();
    initParser();
    int code = 0;
    TopLevel* ast = parseTopLevel();
    if (ast != NULL){
        verifyTopLevel(ast);
        if (checkSemantics()){
            cmplTopLevel(ast);
        }
        else{
            code = 3;   
        }
        disposeAst(ast);
    }
    else {
        code = 3;
        printf("Semantic error placeholder\n");
    }
    disposeLexer();
    safeClose(infile, infilename);
    safeClose(outfile, outfilename);
    return code;
}