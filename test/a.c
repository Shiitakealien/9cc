int main(){
    int a;
    int b;
    int c;
    int n;
    a = 0;
    b = 1;
    for (n = 0; n < 10; n= n + 1){
        c = b;
        b = a + b;
        a = c;
    }
    return a;
}
