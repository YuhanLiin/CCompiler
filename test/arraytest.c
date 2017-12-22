#include <stdio.h>
#include <assert.h>
#include "utils/assert.h"

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

#define checkInitAttrs(arr, sz, allocsz, isNulled) do{\
    assertEqNum(arr.size, sz);\
    assertEqNum(arr.allocatedSize, allocsz);\
    if (isNulled) {assertEqNum(arr.elem, NULL);}\
    else assertNotEqNum(arr.elem, NULL);\
} while (0)

static void testInit(){
    char string[] = "pig";
    Array(char) str;
    // Empty initialization
    arrInit(char)(&str, 0, NULL, NULL);
    checkInitAttrs(str, 0, 0, 1);
    arrDispose(char)(&str);
    // Allocate initial memory but still keep array empty
    arrInit(char)(&str, 4, NULL, NULL);
    checkInitAttrs(str, 0, 4, 0);
    arrExpand(char)(&str);
    checkInitAttrs(str, 4, 4, 0);
    arrDispose(char)(&str);
    // Initialize array at specific size with contents
    arrInit(char)(&str, 4, string, NULL);
    checkInitAttrs(str, 4, 4, 0);
    arrDispose(char)(&str);
}

#define checkArrEquals(sz, carr1, carr2) do{\
    for (size_t __m=0; __m<(sz); __m++){\
        assertEqNum(carr1[__m], carr2[__m]);\
    }\
} while (0)

#define checkContents(type, arr, sz, ...) do{\
    assertEqNum(arr.size, sz);\
    type __contents[sz] = {__VA_ARGS__};\
    checkArrEquals(sz, arr.elem, __contents);\
}while(0)

static void testPushPop(){
    Array(char) str;
    arrInit(char)(&str, 0, NULL, NULL);
    arrPush(char)(&str, 'a');
    arrPush(char)(&str, 'b');
    arrPush(char)(&str, 'd');
    arrPush(char)(&str, 'e');
    checkContents(char, str, 4, 'a', 'b', 'd', 'e');

    arrPop(char)(&str);
    arrPop(char)(&str);
    checkContents(char, str, 2, 'a', 'b');
    arrDispose(char)(&str);
}

static void testExtractInsert(){
    Array(int) arr;
    int a[] = {0,1,2,3};
    arrInit(int)(&arr, 4, a, NULL);
    arrExtract(int)(&arr, 2);
    arrExtract(int)(&arr, 0);
    checkContents(int, arr, 2, 1, 3);
    arrInsert(int)(&arr, 5, 0);
    checkContents(int, arr, 3, 5, 1, 3);
    arrDispose(int)(&arr);
}

int deleted[100];
size_t ind = 0;

void dtr(int c){
    deleted[ind] = c;
    ind++;
}

static void testDispose(){
    Array(int) arr;
    int a[] = {0,1,2,3};
    arrInit(int)(&arr, 4, a, &dtr);
    arrDispose(int)(&arr);
    checkArrEquals(4, deleted, a);
}

int main(int argc, char const *argv[])
{
    testDispose();
    testInit();
    testExtractInsert();
    testPushPop();
    return 0;
}