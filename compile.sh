#!/bin/bash

echo "compiling $1"
rm -f lex.yy.c
rm -f "src/$1.tab.c"
rm -f $1
bison -d "src/$1.y"
lex "src/$1.l"
g++ "src/AST.cpp" "src/SymTable.cpp"  lex.yy.c  "src/$1.tab.c" -o $1
