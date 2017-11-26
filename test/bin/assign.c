unsigned long long assign(unsigned long long x){
    x = 3;
    x += x -= 1;
    x *= 10;
    x /= 5;
}

//Expect 8
int main(){
    return assign(1000);
}