unsigned int loop0(){
    int y = 1;
    while(0){
        y++;
    }
    return y;
}

unsigned int loop1(){
    while (1){
        return 2;
    }
    return 0;
}

unsigned int loop(unsigned int x){
    unsigned int y = 0;
    while(x-y){
        unsigned int x = x;
        y += loop0();
    }
    return y;
}

int dowhile0(){
    int y = 1;
    do y++; while(0);
    return y;
}

int dowhile1(){
    do {
        return 1;
    } while(1);
}

int dowhile(int x){
    int y = 0;
    do{
        int x = x;
        y += dowhile1();
    } while(x-y);
    return y;
}

//Return 42;
int main(){
    return (loop(5) + loop1() + loop(0)) * (dowhile0() + dowhile(4));
}