#include "test/utils/assert.h"

#define KEY char
#define VAL int
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

#define KEY int
#define VAL int
#include "generics/gen_map.h"
#include "generics/gen_map.c"
#undef KEY
#undef VAL

static size_t hashChar(char c){
    return c;
}

static char eqChar(char c, char d){
    return c == d;
}

static int keyCount = 0;
static void keyDtr(char c){
    keyCount++;
}

static int valCount = 0;
static void valDtr(int i){
    valCount++;
}

#define checkSizes(map, sz, allocsz) do{\
    assertEqNum(map.size, sz);\
    assertEqNum(map.allocatedSize, allocsz);\
} while (0);
static void testMapInit(){
    Map(char, int) map;
    assertn0(mapInit(char, int)(&map, 0, &hashChar, &eqChar, NULL, NULL));
    checkSizes(map, 0, 1);
    mapDispose(char, int)(&map);

    assertn0(mapInit(char, int)(&map, 6, &hashChar, &eqChar, NULL, NULL));
    checkSizes(map, 0, 6);
    mapDispose(char, int)(&map);
}

// Checks that a value can be indexed via key after its insertion
#define checkInsert(map, ktype, vtype, key, val) do{\
    assertn0(mapInsert(ktype, vtype)(&(map), key, val));\
    vtype *__vptr = mapFind(ktype, vtype)(&(map), key);\
    assertn0(__vptr);\
    if(__vptr) assertEqNum(*__vptr, val);\
} while (0)

// Checks that the right value has been removed from the map and said value can't be found anymore
#define checkRemove(map, ktype, vtype, key, val) do{\
    const vtype *__vptr = mapRemove(ktype, vtype)(&(map), key);\
    assertn0(__vptr);\
    if (__vptr) assertEqNum(*__vptr, val);\
    assert0(mapFind(ktype, vtype)(&(map), key));\
}while(0)

// Checks that the removal fails
#define checkRemoveFail(map, ktype, vtype, key) assert0(mapRemove(ktype, vtype)(&(map), key))

static void testInsertRemoveFind(){
    Map(char, int) map;
    assertn0(mapInit(char, int)(&map, 0, &hashChar, &eqChar, NULL, NULL));

    assert0(mapFind(char, int)(&map, 'a'));
    checkInsert(map, char, int, 'a', 5);
    checkInsert(map, char, int, 'b', 3);
    checkInsert(map, char, int, 'c', 2);
    checkInsert(map, char, int, 'a', 4);
    assertEqNum(map.size, 3);

    checkRemove(map, char, int, 'a', 4);
    checkRemove(map, char, int, 'b', 3);
    checkRemoveFail(map, char, int, 'd');
    // For load factor calculations this value cannot decrease after removal
    assertEqNum(map.size, 3);

    mapDispose(char, int)(&map);
}

static void testProbing(){
    Map(char, int) map;
    assertn0(mapInit(char, int)(&map, 10, &hashChar, &eqChar, NULL, NULL));
    for (int i=0; i<10; i++){
        checkInsert(map, char, int, i, i*10);
    }
    mapDispose(char, int)(&map);
}

static void testDispose(){
    Map(char, int) map;
    assertn0(mapInit(char, int)(&map, 10, &hashChar, &eqChar, &keyDtr, &valDtr));
    checkInsert(map, char, int, 'a', 5);
    checkInsert(map, char, int, 'b', 3);
    checkRemove(map, char, int, 'b', 3);
    mapDispose(char, int)(&map);
    // On removal, the map disposes the key but gives the value back to the caller to dispose, hence the discrepency
    assertEqNum(keyCount, 2);
    assertEqNum(valCount, 1);
}

int main(int argc, char const *argv[])
{
    testMapInit();
    testInsertRemoveFind();
    testProbing();
    testDispose();
    return 0;
}