#include <stdio.h>
#include <assert.h>

#define KEY char
#define VAL int
#include "../generics/gen_map.h"
#include "../generics/gen_map.c"
#undef KEY
#undef VAL

size_t hashChar(char c){
    return c;
}

char eqChar(char c, char d){
    return c == d;
}

int keyCount = 0;
void keyDtr(char c){
    keyCount++;
}

int valCount = 0;
void valDtr(int i){
    valCount++;
}

int main(int argc, char const *argv[])
{
    char b;
    const int* valptr;
    char key;
    Map(char, int) map;
    b = mapInit(char, int)(&map, 0, &hashChar, &eqChar, NULL, NULL);
    assert(b == 1);
    mapDispose(char, int)(&map);

    mapInit(char, int)(&map, 0, &hashChar, &eqChar, &keyDtr, &valDtr);
    valptr = mapFind(char, int)(&map, 'a');
    assert(valptr == NULL);
    b = mapInsert(char, int)(&map, 'a', 5);
    assert(b == 1);
    valptr = mapFind(char, int)(&map, 'a');
    assert(*valptr == 5);
    mapInsert(char, int)(&map, 'b', 3);
    mapInsert(char, int)(&map, 'c', 2);
    valptr = mapFind(char, int)(&map, 'a');
    assert(*valptr == 5);
    valptr = mapFind(char, int)(&map, 'b');
    assert(*valptr == 3);
    valptr = mapFind(char, int)(&map, 'c');
    assert(*valptr == 2);
    mapInsert(char, int)(&map, 'a', 4);
    valptr = mapFind(char, int)(&map, 'a');
    assert(*valptr == 4);
    assert(map.size == 3);

    valptr = mapRemove(char, int)(&map, 'a', &key);
    assert(*valptr == 4);
    assert(key == 'a');
    assert(map.size == 3);
    valptr = mapFind(char, int)(&map, 'a');
    assert(valptr == NULL);
    
    valptr = mapFind(char, int)(&map, 'b');
    assert(*valptr == 3);
    valptr = mapFind(char, int)(&map, 'c');
    assert(*valptr == 2);

    mapDispose(char, int)(&map);
    assert(keyCount == 2);
    assert(valCount == 2);
    return 0;
}