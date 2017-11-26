int neg(int x){
    -x;
    //Ensure this has no side effects
    return x;
}

int pp(int x){
    return x++ + x;
}
int mm(int x){
    return x-- + x;
}

int lpp(int x){
    return ++x + x;
}
int lmm(int x){
    return --x + x;
}

//Expect 48
int main(){
    return (-4 + neg(5)) * (pp(1) + mm(2)) * (lpp(1) + lmm(3));
}