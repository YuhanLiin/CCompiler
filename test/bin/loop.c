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

//Return 7;
int main(){
    return loop(5) + loop1() + loop(0);
}