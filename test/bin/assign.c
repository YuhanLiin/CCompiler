unsigned long long assign(unsigned long long x){
    x = 3;
    x += x -= 1;
    x *= 10;
    x /= 5;
    return x;
}

int var(){
    int a;
    int b = 11;
    a = 5;
    return a + b;
}

//Expect 8
int main(){
    return var() - assign(1000);
}