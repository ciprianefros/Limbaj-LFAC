#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <vector>
#include <string>
#include "SymTable.h"
#include "AST.h"


ParamList currentParams;
VarInfo currentFunction;
FuncInfo currentVariable;
ClassInfo currentClass;

using namespace std;

extern int yylineno;



#endif