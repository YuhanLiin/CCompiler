#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MOCKBUFSIZ 4000

//Mock stdin/filein
char_t input[MOCKBUFSIZ];
size_t ilen = 0;
//Mock stdout/fileout
char_t output[MOCKBUFSIZ];
size_t olen = 0;
//Mock stderr
char_t errorstr[MOCKBUFSIZ];
size_t elen = 0;

//Asserts positivity/success
static int above0(int x){
    assert(x >= 0 && "Why the hell is sprintf failing???");
    return x;
}

//Mock out the IO functions
char_t consumeNext(){
    char_t c = input[ilen++];
    return c;
}

void writeError(size_t line, size_t pos, char_t* message, ...){
    elen += above0(sprintf(&errorstr[elen], "%d:%d ", line, pos));
    va_list args;
    va_start(args, message);
    elen += above0(vsprintf(&errorstr[elen], message, args));
    va_end(args);
    elen += above0(sprintf(&errorstr[elen], "\n"));
}

//Output string to output array
void emitOut(const char* format, ...) {
    va_list args;
    va_start(args, format);
    olen += above0(vsprintf(&output[olen], format, args));
    va_end(args);
}

void ioSetup(const char_t* str){
    strcpy(input, str); //Might need to be adjusted for bigger char types
    ilen = 0;
    memset(output, 0, MOCKBUFSIZ*sizeof(char_t));  //Also needs to change for widechar
    olen = 0;
    memset(errorstr, 0, MOCKBUFSIZ*sizeof(char_t));
    elen = 0;
}