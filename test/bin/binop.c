int add5(int a, int b){
    return a + b + 5;
}

int minus(int a){
    return 50 - add5(1, 2) - a;
}

int multi(int a, int b){
    return a * b;
}
unsigned int umulti(int a, int b){
    return a * b;
}

int div(int x){
    return 100 / x;
}
unsigned int udiv(int x){
    return 100U /x;
}

int main(){
    if (add5(-1, -2) == 2){
        if (minus(3) == 39){
            if (multi(-2, -2) == 4){
                if(umulti(1, 0) == 0){
                    if (div(-5) == -20){
                        if (udiv(11) == 9){
                            return 0;
                        }
                        return 1;
                    }
                    return 2;
                }
                return 3;
            }
            return 4;
        }
        return 5;
    }
    return 6;
}