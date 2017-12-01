int b(int x);

int p(int p1, int p2, int p3, int p4, int p5){
    return b(p5);
}

int b(int x){
    int x;
    return p(x, 2, 3, 4, 5);
}