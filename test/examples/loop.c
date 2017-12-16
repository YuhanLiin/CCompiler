int loop(int x){
    while(x){
        int y;
        loop(y);
        x+=y;
    }
}

int condition(int a, int b){
    int x = a == b;
    int y = a > b;
    int z = a < b;
    return x + y + z;
}