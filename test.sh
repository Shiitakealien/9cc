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
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 "12 + 34 - 5   ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 6 "a=3;b=2;c=1;a+b+c;"
try 6 "a=b=c=2;a+b+c;"
try 6 "a1=b=c=2;a1+b+c;"
try 6 "a1bbb=b=c=2;a1bbb+b+c;"
try 27 "A1bbb=b=c=9;A1bbb+b+c;"
echo OK
