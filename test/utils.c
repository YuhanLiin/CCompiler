char strEq(Array(char_t)* string, const char_t * cstr){
    for (size_t i=0; i<string->size; i++){
        if (cstr[i] == '\0' || cstr[i] != string->elem[i]){
            return 0;
        }
    }
    return 1;
}

char doubleEq(const double a, const double b){
    double diff = a - b;
    static double tolerance = 0.0001;
    return -tolerance < diff || diff < tolerance;
}