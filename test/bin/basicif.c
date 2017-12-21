int two(){
    return 2;
}

// This is the type of code I'll be using to test return values. Need to prove this is correct first
int main(){
    if (0 != 0) return 0;
    if (1 != 3){
        if (2 == two()){
            return 1;
        }
        return 2;
    }
    return 3;
}