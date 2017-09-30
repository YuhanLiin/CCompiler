#include "utils.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <stdio.h>
#include <stdargs.h>

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