int add(int a, int b){
    return a + b + 5;
}

int minus(int a){
    return 50 - add(1, 2) - a;
}

int multi(int a, int b, int c, int d, int f){
    return a + b * (c + d);
}
// unsigned int imulti(int a, int b, unsigned int c, unsigned int d, int f){
//     return a + b * (c + d);
// }

int div(){
    return 100 / multi(0, 5, add(1, 1), 3, minus(1));
}
unsigned int idiv(){
    return 100U / multi(0, 5, add(1, 1), 3, minus(1));
}

//Should return 46
int main(){
    return add(1, minus(div()));
}