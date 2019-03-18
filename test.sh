#!/bin/bash
try(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -c tmp.s
    gcc -o tmp tmp.o func_define.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

cat func_define.c
try 0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 "12 + 34 - 5   ;"
try 47 "5+6*7;"
try 77 "(5+6)*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 6 "a=3;b=2;c=1;a+b+c;"
try 21 "a=3+4+5+6;b=2;c=1;a+b+c;"
try 6 "a=b=c=2;a+b+c;"
try 6 "a1=b=c=2;a1+b+c;"
try 6 "a1bbb=b=c=2;a1bbb+b+c;"
try 27 "A1bbb=b=c=9;A1bbb+b+c;"
try 1 "2==2;"
try 0 "2==3;"
try 0 "2!=2;"
try 1 "2!=3;"
try 1 "a=2;b=2;a==b;"
try 0 "a=2;b=3;a==b;"
try 1 "a=2;b=3;a+1==b;"
try 1 "a=2;b=4;a*2==b;"
try 0 "a=2;b=3;a+1!=b;"
try 0 "a=2;b=4;a*2!=b;"
try 1 "a=2;b=3;b==a+1;"
try 1 "a=2;b=4;b==a*2;"
try 0 "a=2;b=2;a!=b;"
try 1 "a=2;b=3;a!=b;"
try 3 "bar(); foo(    );"
try 2 "foo(); bar(    );"
try 5 "a=foo();b=bar();a+b;"
try 2 "foo0(1);"
try 2 "a=1;foo0(a);"
try 20 "foo1(1,10);"
try 20 "a=1;foo1(a,10);"
try 3 "a=1;foo2(3,10);"
try 13 "a=1;foo3(3,10);"
try 7 "a=1;foo4(10,3);"
echo OK
