#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <vector>
#include <string>
#include "SymTable.h"
#include "AST.h"

void yyerror(const char * s);

FuncInfo currentFunction;
VarInfo currentVariable;
ClassInfo currentClass;
vector<VarInfo> currentParams;
vector<short> currentArraySizes;
SymTable* current_p;

class SymTable* current;
vector<SymTable*> tables;
SymTable* globalTable = new SymTable("global");
SymTable* currentTable = globalTable;
SymTable* p;

using namespace std;

extern int yylineno;
extern int errorCount;

bool exists_or_add(const string& name, bool is_array) {
    SymTable* temp = currentTable;
    while(temp != nullptr) {
        if (temp->existsId(name)) {
            errorCount++; 
            yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
            return false;
        } 
        temp = temp->prev;
    }
    if(is_array == true) {
        currentVariable.type.isArray = true;
        currentVariable.type.arraySizes = currentArraySizes;
    } else {
        currentVariable.type.isArray = false;
    }
    currentVariable.name = name;
    currentVariable.value.setType(currentVariable.type.typeName);
    currentTable->addVar(currentVariable);
    currentArraySizes.clear();
    currentVariable = VarInfo();
    return true;
}

#endif