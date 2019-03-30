#!/bin/bash
err_cnt=0
try(){
    expected="$1"
    input="$2"

    ./icc "$input" > tmp.s
    gcc -o tmp tmp.s tmp-test.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "OK : $input => $actual"
    else
        echo "NG : $input : $expected expected, but got $actual"
        err_cnt=$((err_cnt+1))
    fi
}

cat <<EOF | gcc -xc -c -o tmp-test.o -
int *alloc4(int x0,int x1,int x2,int x3) {
  static int arr[4];
  arr[0] = x0;
  arr[1] = x1;
  arr[2] = x2;
  arr[3] = x3;
  return arr;
}
int *alloc4a(int x0,int x1,int x2,int x3) {
  static int arr[4];
  arr[0] = x0;
  arr[1] = x1;
  arr[2] = x2;
  arr[3] = x3;
  return arr+3;
}
EOF

for x in ex/*.c
do
    a=`cat $x`
    echo "$a"
    ./icc "$a" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    echo "$?"
done

try 0   "int main(){;;;return 0;}"
try 0   "int main(){return 0;}"
try 42  "int main(){return 42;}"
try 21  "int main(){return 5+20-4;}"
try 41  "int main(){return 12 + 34 - 5   ;}"
try 47  "int main(){return 5+6*7;}"
try 77  "int main(){return (5+6)*7;}"
try 15  "int main(){return 5*(9-6);}"
try 4   "int main(){return (3+5)/2;}"
try 3   "int main(){int a;a=3;return a;}"
try 7   "int main(){int a;a=3;int b;b=4;return a+b;}"
try 9   "int main(){int a;int b;int c;a=3;b=3;c=3;return a+b+c;}"
try 6   "int main(){int a;int b;int c;a=3;b=2;c=1;return a+b+c;}"
try 21  "int main(){int a;a=3+4+5+6;int b;b=2;int c;c=1;return a+b+c;}"
try 9   "int main(){int a;int b;int c;a=1;b=2;c=4;return a+b*c;}"
try 6   "int main(){int a1;int b;int c;a1=b=c=2;return a1+b+c;}"
try 6   "int main(){int a1bbb;int b;int c;a1bbb=b=c=2;return a1bbb+b+c;}"
try 27  "int main(){int A1bbb;int b;int c;A1bbb=b=c=9;return A1bbb+b+c;}"
try 1   "int main(){return 2==2;}"
try 0   "int main(){return 2==3;}"
try 0   "int main(){return 2!=2;}"
try 1   "int main(){return 2!=3;}"
try 0   "int main(){return 1>2;}"
try 0   "int main(){return 2>2;}"
try 1   "int main(){return 3>2;}"
try 0   "int main(){return 3>=4;}"
try 1   "int main(){return 3>=3;}"
try 1   "int main(){return 3>=2;}"
try 1   "int main(){return 1<2;}"
try 0   "int main(){return 2<2;}"
try 0   "int main(){return 3<2;}"
try 1   "int main(){int a;int b;a=3;b=4;return a<=b;}"
try 1   "int main(){int a;int b;a=3;b=3;return a<=b;}"
try 0   "int main(){int a;int b;a=3;b=2;return a<=b;}"
try 0   "int main(){int a;int b;a=1;b=2;return a >b;}"
try 0   "int main(){int a;int b;a=2;b=2;return a >b;}"
try 1   "int main(){int a;int b;a=3;b=2;return a >b;}"
try 0   "int main(){int a;int b;a=3;b=4;return a>=b;}"
try 1   "int main(){int a;int b;a=3;b=3;return a>=b;}"
try 1   "int main(){int a;int b;a=3;b=2;return a>=b;}"
try 1   "int main(){int a;int b;a=1;b=2;return a <b;}"
try 0   "int main(){int a;int b;a=2;b=2;return a <b;}"
try 0   "int main(){int a;int b;a=3;b=2;return a <b;}"
try 1   "int main(){int a;int b;a=3;b=4;return a<=b;}"
try 1   "int main(){int a;int b;a=3;b=3;return a<=b;}"
try 0   "int main(){int a;int b;a=3;b=2;return a<=b;}"
try 1   "int main(){int a;int b;a=2;b=2;return a==b;}"
try 0   "int main(){int a;int b;a=2;b=3;return a==b;}"
try 1   "int main(){int a;int b;a=2;b=3;return a+1==b;}"
try 1   "int main(){int a;int b;a=2;b=4;return a*2==b;}"
try 0   "int main(){int a;int b;a=2;b=3;return a+1!=b;}"
try 0   "int main(){int a;int b;a=2;b=4;return a*2!=b;}"
try 1   "int main(){int a;int b;a=2;b=3;return b==a+1;}"
try 1   "int main(){int a;int b;a=2;b=4;return b==a*2;}"
try 0   "int main(){int a;int b;a=2;b=2;return a!=b;}"
try 1   "int main(){int a;int b;a=2;b=3;return a!=b;}"
try 3   "int foo(){return 3;}int bar(){return 2;} int main(){bar();return  foo(    );}"
try 2   "int foo(){return 3;}int bar(){return 2;} int main(){foo();return  bar(    );}"
try 5   "int foo(){return 3;}int bar(){return 2;} int main(){int a;int b;a=foo();b=bar();return a+b;}"
try 2   "int foo0(int x){return 2;}int main(){return foo0(1);}"
try 2   "int foo0(int x){return 2;}int main(){int a;a=1;return foo0(a);}"
try 20  "int foo1(int x,int y){return 20;}int main(){return foo1(1,10);}"
try 20  "int foo1(int x,int y){return 20;}int main(){int a;a=1;return foo1(a,10);}"
try 3   "int foo2(int x,int y){return x;}int main(){int a;a=1;return foo2(3,10);}"
try 10  "int foo2(int x,int y){return y;}int main(){int a;a=1;return foo2(3,10);}"
try 13  "int foo3(int x,int y){return x+y;}int main(){int a;a=1;return foo3(3,10);}"
try 7   "int foo4(int x,int y){return x-y;}int main(){int a;a=1;return foo4(10,3);}"
try 5   "int foo3(int x,int y){return x+y;}int main(){int a;int b;a=1;b=4;return foo3(a,b);}"
try 6   "int foo3(int x,int y){return x+y;}int main(){int a;int b;a=1;b=4;return foo3(a,a+b);}"
try 1   "int foo3(int x,int y){return x;}int main(){int a;int b;a=1;b=4;return foo3(a,a+b);}"
try 5   "int foo3(int x,int y){return y;}int main(){int a;int b;a=1;b=4;return foo3(a,a+b);}"
try 5   "int foo3(int x,int y){return x;}int main(){int a;int b;a=1;b=4;return foo3(a+b,a);}"
try 1   "int foo3(int x,int y){return y;}int main(){int a;int b;a=1;b=4;return foo3(a+b,a);}"
try 1   "int foo3(int x,int y){return x;}int main(){return foo3(1,1+4);}"
try 5   "int foo3(int x,int y){return y;}int main(){return foo3(1,1+4);}"
try 5   "int foo3(int x,int y){return x;}int main(){return foo3(1+4,4);}"
try 4   "int foo3(int x,int y){return y;}int main(){return foo3(1+4,4);}"
try 4   "int foo3(int x,int y){return y;}int main(){int a;a=foo3(1+4,4);return a;}"
try 3   "int f(int n1,int n0){return n1+n0;}int main(){return f(f(1,1),1);}"
try 1   "int a(){return 1;}int main(){return a();}"
try 2   "int a(){return 2;}int main(){return a();}"
try 10  "int a(){return 2;}int b(){return 3;}int main(){int c;c=a()+b();return 2*c;}"
try 7   "int a(){return 2;}int b(){return 3;}int main(){int c;c=a()+b();return (2+c);}"
try 5   "int a(int x,int y){return x+y;}int main(){return a(2,3);}"
try 6   "int a(int x,int y){return x*y;}int main(){return a(2,3);}"
try 3   "int a(int x,int y){return y;}int main(){return a(2,3);}"
try 2   "int a(int x,int y){return x;}int main(){return a(2,3);}"
try 2   "int a(int x,int y,int z){return x;}int main(){return a(2,3,4);}"
try 3   "int a(int x,int y,int z){return y;}int main(){return a(2,3,4);}"
try 4   "int a(int x,int y,int z){return z;}int main(){return a(2,3,4);}"
try 2   "int a(int x,int y,int z, int w){return x;}int main(){return a(2,3,4,5);}"
try 3   "int a(int x,int y,int z, int w){return y;}int main(){return a(2,3,4,5);}"
try 4   "int a(int x,int y,int z, int w){return z;}int main(){return a(2,3,4,5);}"
try 5   "int a(int x,int y,int z, int w){return w;}int main(){return a(2,3,4,5);}"
try 17  "int a(int x,int y,int z){return x+y*z;}int main(){return a(2,3,5);}"
try 1   "int main(){if(1)return 1;}"
try 2   "int main(){if(0)return 1;else return 2;}"
try 6   "int main(){int a;a=0;while(a!=6)a=a+1;return a;}"
try 1   "int main(){int a;{a=1;}return a;}"
try 10  "int main(){int a;a=0;while(a!=10){a=a+1;}return a;}"
try 10  "int main(){int a;a=0;while(a!=10){a=a+1;a=a-2;a=a+2;}return a;}"
try 11  "int main(){int a;int b;a=3;b=1;for(a=0;a<=10;a=a+1)b=a;return a;}"
try 11  "int main(){int a;int b;a=3;b=1;for(;a<=10;){b=a;a=a+1;}return a;}"
try 13  "int main(){int *a;int b;a=&b;b=13;return *a;}"
try 13  "int main(){int *a;int b;a=&b;*a=13;return b;}"
try 15  "int main(){int **a;int *b;int c;a=&b;b=&c;c=15;return **a;}"
try 19  "int main(){int **a;int *b;int c;a=&b;*a=&c;c=19;return *b;}"
try 1   "int main(){int *a;a=alloc4(1,2,3,4); return *a;}"
try 1   "int main(){int *a;a=alloc4(1,2,3,4);  a=a+0;return *a;}"
try 2   "int main(){int *a;a=alloc4(1,2,3,4);  a=a+1;return *a;}"
try 3   "int main(){int *a;a=alloc4(1,2,3,4);  a=a+2;return *a;}"
try 4   "int main(){int *a;a=alloc4(1,2,3,4);  a=a+3;return *a;}"
try 4   "int main(){int *a;a=alloc4a(1,2,3,4); a=a-0;return *a;}"
try 3   "int main(){int *a;a=alloc4a(1,2,3,4); a=a-1;return *a;}"
try 2   "int main(){int *a;a=alloc4a(1,2,3,4); a=a-2;return *a;}"
try 1   "int main(){int *a;a=alloc4a(1,2,3,4); a=a-3;return *a;}"
try 1   "int main(){int *a;a=alloc4(1,2,3,4);  return *(a);}"
try 1   "int main(){int *a;a=alloc4(1,2,3,4);  return *(a+0);}"
try 2   "int main(){int *a;a=alloc4(1,2,3,4);  return *(a+1);}"
try 3   "int main(){int *a;a=alloc4(1,2,3,4);  return *(a+2);}"
try 4   "int main(){int *a;a=alloc4(1,2,3,4);  return *(a+3);}"
try 4   "int main(){int *a;a=alloc4a(1,2,3,4); return *(a-0);}"
try 3   "int main(){int *a;a=alloc4a(1,2,3,4); return *(a-1);}"
try 2   "int main(){int *a;a=alloc4a(1,2,3,4); return *(a-2);}"
try 1   "int main(){int *a;a=alloc4a(1,2,3,4); return *(a-3);}"
echo "Error Count => $err_cnt"
