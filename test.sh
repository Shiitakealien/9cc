#!/bin/bash
try(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "input is $input : $expected expected, but got $actual"
        exit 1
    fi
}
try 0   "main(){return 0;}"
try 42  "main(){return 42;}"
try 21  "main(){return 5+20-4;}"
try 41  "main(){return 12 + 34 - 5   ;}"
try 47  "main(){return 5+6*7;}"
try 77  "main(){return (5+6)*7;}"
try 15  "main(){return 5*(9-6);}"
try 4   "main(){return (3+5)/2;}"
try 6   "main(){a=3;b=2;c=1;return a+b+c;}"
try 21  "main(){a=3+4+5+6;b=2;c=1;return a+b+c;}"
try 6   "main(){a=b=c=2;return a+b+c;}"
try 6   "main(){a1=b=c=2;return a1+b+c;}"
try 6   "main(){a1bbb=b=c=2;return a1bbb+b+c;}"
try 27  "main(){A1bbb=b=c=9;return A1bbb+b+c;}"
try 1   "main(){return 2==2;}"
try 0   "main(){return 2==3;}"
try 0   "main(){return 2!=2;}"
try 1   "main(){return 2!=3;}"
try 1   "main(){a=2;b=2;return a==b;}"
try 0   "main(){a=2;b=3;return a==b;}"
try 1   "main(){a=2;b=3;return a+1==b;}"
try 1   "main(){a=2;b=4;return a*2==b;}"
try 0   "main(){a=2;b=3;return a+1!=b;}"
try 0   "main(){a=2;b=4;return a*2!=b;}"
try 1   "main(){a=2;b=3;return b==a+1;}"
try 1   "main(){a=2;b=4;return b==a*2;}"
try 0   "main(){a=2;b=2;return a!=b;}"
try 1   "main(){a=2;b=3;return a!=b;}"
try 3   "foo() {return 3;}bar() {return 2;} main(){bar();return  foo(    );}"
try 2   "foo() {return 3;}bar() {return 2;} main(){foo();return  bar(    );}"
try 5   "foo() {return 3;}bar() {return 2;} main(){a=foo();b=bar();return a+b;}"
try 2   "foo0(x){return 2;}main(){return foo0(1);}"
try 2   "foo0(x){return 2;}main(){a=1;return foo0(a);}"
try 20  "foo1(x,y){return 20;}main(){return foo1(1,10);}"
try 20  "foo1(x,y){return 20;}main(){a=1;return foo1(a,10);}"
try 3   "foo2(x,y){return x;}main(){a=1;return foo2(3,10);}"
try 13  "foo3(x,y){return x+y;}main(){a=1;return foo3(3,10);}"
try 7   "foo4(x,y){return x-y;}main(){a=1;return foo4(10,3);}"
try 5   "foo3(x,y){return x+y;}main(){a=1;b=4;return foo3(a,b);}"
try 6   "foo3(x,y){return x+y;}main(){a=1;b=4;return foo3(a,a+b);}"
try 3   "f(n1,n0){return n1+n0;}main(){return f(f(1,1),1);}"
try 1   "a(){return 1;}main(){return a();}"
try 2   "a(){return 2;}main(){return a();}"
try 10  "a(){return 2;}b(){return 3;}main(){c=a()+b();return 2*c;}"
try 5   "a(x,y){return x+y;}main(){return a(2,3);}"
try 6   "a(x,y){return x*y;}main(){return a(2,3);}"
try 3   "a(x,y){return y;}main(){return a(2,3);}"
try 2   "a(x,y){return x;}main(){return a(2,3);}"
try 17  "a(x,y,z){return x+y*z;}main(){return a(2,3,5);}"
try 1   "main(){if(1)return 1;}"
try 2   "main(){if(0)return 1;else return 2;}"
try 6   "main(){a=0;while(a!=6)a=a+1;return a;}"

echo OK
