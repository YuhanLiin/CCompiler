#include "array.h"
#include "utils.h"

#define TYPE char_t
#include "generics/gen_array.c"
#undef TYPE 

#define TYPE vptr
#include "generics/gen_array.c"
#undef TYPE 

#define TYPE size_t
#include "generics/gen_array.c"
#undef TYPE

//Allocate and return new cstring from a char array
char_t* toCstring(const Array(char_t)* str){
    New(char_t, cstr, str->size+1)
    memcpy(cstr, str->elem, sizeof(char_t)*(str->size+1));
    cstr[str->size] = 0;
    return cstr;
}