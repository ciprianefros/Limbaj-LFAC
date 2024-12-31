#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <vector>
#include <string>
#include "SymTable.h"
#include "AST.h"


FuncInfo currentFunction;
VarInfo currentVariable;
ClassInfo currentClass;
vector<VarInfo> currentParams;

using namespace std;

extern int yylineno;



#endif