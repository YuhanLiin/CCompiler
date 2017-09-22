#include <stdio.h>
#include <assert.h>

#define TYPE char
#include "../generics/gen_array.h"
#undef TYPE 
#define TYPE char
#include "../generics/gen_array.c"
#undef TYPE 

#define TYPE int
#include "../generics/gen_array.h"
#undef TYPE 
#define TYPE int
#include "../generics/gen_array.c"
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
    initArr(char)(&str, 0, NULL, NULL);
    disposeArr(char)(&str); //See if null array is safe to dispose freely
    pushArr(char)(&str, 'a');
    pushArr(char)(&str, 'b');
    pushArr(char)(&str, 'd');
    pushArr(char)(&str, 'e');
    popArr(char)(&str);
    popArr(char)(&str);
    assert(str.size == 2);
    assert(str.elem[0] == 'a');
    assert(str.elem[1] == 'b');
    Array(char) str2;
    copyArr(char)(&str2, &str);
    assert(str2.size == 2);
    assert(str2.elem[0] == 'a');
    assert(str2.elem[1] == 'b');
    disposeArr(char)(&str);
    disposeArr(char)(&str2);

    Array(int) arr;
    int a[] = {0,1,2,3};
    initArr(int)(&arr, 4, a, &testDtr);
    extractArr(int)(&arr, 2);
    extractArr(int)(&arr, 0);
    insertArr(int)(&arr, 5, 0);
    assert(arr.size == 3);
    assert(arr.elem[0] == 5);
    assert(arr.elem[1] == 1);
    assert(arr.elem[2] == 3);
    disposeArr(int)(&arr);
    assert(deleted[0] == 5);
    assert(deleted[1] == 1);
    assert(deleted[2] == 3);
    return 0;
}