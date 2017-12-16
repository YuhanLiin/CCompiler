// expect y+3
int add3(int y){
    while(1){
        int x = 0;
        while(5 - x){
            x++;
            continue;
            y++;
        }
        break;
    }
    return y+3;
}

// expect y
int cornercase(int y){
    do {
        while (0){
            continue;
            break;
        }
        while (1){
            break;
            continue;
        }
        break;
        continue;
    } while(1);
    do {
        break;
        continue;
    } while(0);
    return y;
}

// expect 4
int four(){
    int y = 1;
    do {
        do {
            y *= 2;
            continue;
            break;
        } while(4-y);
    } while(0);
    return y;
}

// expect 5 + 1 + 4 = 10
int main(){
    return add3(2) + cornercase(1) + four();
}