int add(int a, int b){
    return a + b + 5;
}

int minus(int a){
    return 50 - add(1, 2) - a;
}

int main(){
    return minus(add(1, 1));
}