int loop(int x){
    while(x){
        int y;
        loop(y);
        x+=y;
    }
}