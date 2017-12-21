unsigned long long assign(unsigned long long x){
    x = 3;
    return x;
}

int assignAddMinus(int x){
    return x += x -= 1;
}

int assignDivMul(int x){
    x *= 11;
    x /= 5;
    return x;
}

int var(){
    int a;
    int b = 11;
    a = 5;
    //See if the call corrupts the variable
    assign(a);
    return a + b;
}

//Expect 8
int main(){
    if (var() == 16){
        if (assign(1000) == 3){
            if (assignAddMinus(3) == 4){
                if (assignDivMul(1) == 2){
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