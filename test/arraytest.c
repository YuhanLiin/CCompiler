#include <stdio.h>
#include <assert.h>

#define TYPE char
#include "generics/gen_array.h"
#undef TYPE 
#define TYPE char
#include "generics/gen_array.c"
#undef TYPE 

#define TYPE int
#include "generics/gen_array.h"
#undef TYPE 
#define TYPE int
#include "generics/gen_array.c"
#undef TYPE 

int deleted[100];
size_t ind = 0;

void testDtr(int c){
    deleted[ind] = c;
    ind++;
}

int main(int argc, char const *argv[])
{
    Array(char) str; 
    arrInit(char)(&str, 0, NULL, NULL);
    arrDispose(char)(&str); //See if null array is safe to dispose freely
    arrPush(char)(&str, 'a');
    arrPush(char)(&str, 'b');
    arrPush(char)(&str, 'd');
    arrPush(char)(&str, 'e');
    arrPop(char)(&str);
    arrPop(char)(&str);
    assert(str.size == 2);
    assert(str.elem[0] == 'a');
    assert(str.elem[1] == 'b');
    Array(char) str2;
    arrCopy(char)(&str2, &str);
    assert(str2.size == 2);
    assert(str2.elem[0] == 'a');
    assert(str2.elem[1] == 'b');
    arrDispose(char)(&str);
    arrDispose(char)(&str2);

    Array(int) arr;
    int a[] = {0,1,2,3};
    arrInit(int)(&arr, 4, a, &testDtr);
    arrExtract(int)(&arr, 2);
    arrExtract(int)(&arr, 0);
    arrInsert(int)(&arr, 5, 0);
    assert(arr.size == 3);
    assert(arr.elem[0] == 5);
    assert(arr.elem[1] == 1);
    assert(arr.elem[2] == 3);
    arrDispose(int)(&arr);
    assert(deleted[0] == 5);
    assert(deleted[1] == 1);
    assert(deleted[2] == 3);
    return 0;
}