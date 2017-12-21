int ne(int a, int b){
    if (a != b){
        a = 1;
    }
    else a = 2;
    return a;
}

int ge(int a, int b){
    if (a >= b){
        return a;
    }
    return 0;
}

unsigned int le(unsigned int a, unsigned int b){
    if (a <= b){
        return a;
    }
    return 0;
}

int lt(int x, int y){
    if (x<y) return x;
    else return 0;
}

unsigned int gt(unsigned int x, unsigned int y){
    if (x>y) return x;
    else return 0;
}

int dangling(int a, int b){
    if (a == 1) if (b == 2) return a; else return b;
    return 0;
}

int elseif(int a){
    if (a > 10) return a;
    else if (a < 8) return a - 8;
    else return 1;
}

int main(){
    if (ne(-1, -1) != 2) return 0;
    if (ne(0, 4) != 1) return 1;
    if (ge(5, 5) != 5) return 2;
    if (ge(-2, -4) != -2) return 3;
    if (ge(0, 4) != 0) return 4;
    if (le(5, 5) != 5) return 5;
    if (le(10, 4) != 0) return 6;
    if (le(1, -10) != 1) return 7; //-10 unsigned is actually huge, so this test should be positive
    if (lt(1, -10) != 0) return 8;
    if (lt(3, 3) != 0) return 9;
    if (lt(-1, 0) != -1) return 10;
    if (gt(1, 3) != 0) return 11;
    if (gt(-10, 10) != -10) return 12; //Same here
    if (dangling(1, 3) != 3) return 13;
    if (dangling(2, 2) != 0) return 14;
    if (elseif(7) != -1) return 15;
    if (elseif(9) != 1) return 16;
    return 17; 
}