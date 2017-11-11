#include <stdarg.h>
#include <stdio.h>
#include "utils.h"
#include "lexer/lexer.h"

static FILE* infile;
static FILE* outfile;

void emitAsm(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(outfile, format, args);
    va_end(args);
}

char_t consumeNext(){
    int c = fgetc(infile);
    if (c == EOF){
        return End;
    }
    return c;
}

static void safeClose(FILE* file, const char* filename){
    if (file == NULL) return;
    if (fclose(file) != 0){
        fprintf(stderr, "Warning: File %s did not close successfully\n", filename);
    }
}

void closeFiles(const char* infilename, const char* outfilename){
    safeClose(infile, infilename);
    safeClose(outfile, outfilename);
}

void openFiles(const char* infilename, const char* outfilename){
    infile = fopen(infilename, "r");
    outfile = fopen(outfilename, "w");
    if (infile == NULL || outfile == NULL){
        if (infile == NULL){
            fprintf(stderr, "Error: Input file %s couldn't be opened.\n", infilename);
        }
        if (outfile == NULL){
            fprintf(stderr, "Error: Output file %s couldn't be opened.\n", outfilename);
        }
        closeFiles(infilename, outfilename);
        exit(2);
    }
}