#include <assert.h>
#include <string.h>
#include <stdio.h>

char_t input[BUFSIZ];
size_t ilen = 0;
char_t output[BUFSIZ];
size_t olen = 0;
char_t errorstr[BUFSIZ];
size_t elen = 0;

char_t consumeNext(){
    char_t c = input[ilen++];
    return c;
}

//Output string to output array
#define outprint(...) do {{ \
    int __ = sprintf(&output[olen], __VA_ARGS__);    \
    assert(__>=0); olen += __;}} while(0)

void ioSetup(const char_t* str){
    strcpy(input, str); //Might need to be adjusted for bigger char types
    ilen = 0;
    memset(output, 0, BUFSIZ*sizeof(char_t));  //Also needs to change for widechar
    olen = 0;
    memset(errorstr, 0, BUFSIZ*sizeof(char_t));
    setbuf(stderr, errorstr);
}