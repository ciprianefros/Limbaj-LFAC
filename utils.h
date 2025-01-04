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

string currentClassName;

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

bool checkParams(const string& name, SymTable functionScope)
{
    int contor = 0;

    // Debug: Afișăm numărul de parametri așteptat și numărul de parametri actuali
    std::cout << "Checking parameters for function: " << name << std::endl;
    std::cout << "Expected params: " << functionScope.funcids[name].params.size() << ", Given params: " << currentParams.size() << std::endl;

    if(functionScope.funcids[name].params.size() != currentParams.size())
    {
        yyerror(("Not having the same number of params. Error at line: " + std::to_string(yylineno)).c_str());
        return false;
    }

    // Comparăm tipurile parametrilor
    for(auto param : functionScope.funcids[name].params)
    {
        std::cout << "Expected param type: " << param.type.typeName << ", Given param type: " << currentParams[contor].type.typeName << std::endl;
        if(param.type.typeName != currentParams[contor].type.typeName)
        {
            std::cout << "Type mismatch at parameter " << contor + 1 << std::endl;
            return false;
        }
        contor++;
    }
    return true;
}


bool checkFunction(const string& name)
{
    SymTable* temp = currentTable;

    // Debug: Afișăm ce funcție căutăm
    std::cout << "Checking function: " << name << std::endl;

    while(temp != nullptr) 
    {
        // Verificăm dacă funcția există în tabelul curent
        if (!temp->existsFunc(name)) 
        {
            std::cout << "Function not found in current scope. Checking previous scope..." << std::endl;
            temp = temp->prev; 
            continue;
        }
        else
        {
            std::cout << "Function found in current scope." << std::endl;

            // Verificăm parametrii funcției
            if(checkParams(name, *temp))
            {
                std::cout << "Function and parameters match." << std::endl;
                return true;
            }
            else
            {
                errorCount++;
                yyerror(("Function's params are not the same type. Error at line: " + std::to_string(yylineno)).c_str());
                return false;
            }
        } 
    }

    // Dacă am ajuns aici, funcția nu a fost găsită în niciun tabel de simboluri
    errorCount++;
    yyerror(("Function was not declared. Error at line: " + std::to_string(yylineno)).c_str());

    currentParams.clear();
    return false;
}

bool checkMethod()
{
    SymTable* temp = currentTable;

    std::cout << "Checking function: " << currentClassName << std::endl;
    

    while(temp != nullptr) 
    {
        // Verificăm dacă funcția există în tabelul curent
        if (!temp->existsFunc(currentClassName)) 
        {
            std::cout << "Function not found in current scope. Checking previous scope..." << std::endl;
            temp = temp->prev; 
            continue;
        }
        else
        {
            std::cout << "Function found in current scope." << std::endl;

            // Verificăm parametrii funcției
            if(checkParams(currentClassName, *temp))
            {
                std::cout << "Function and parameters match." << std::endl;
                return true;
            }
            else
            {
                errorCount++;
                yyerror(("Function's params are not the same type. Error at line: " + std::to_string(yylineno)).c_str());
                return false;
            }
        } 
    }

    // Dacă am ajuns aici, funcția nu a fost găsită în niciun tabel de simboluri
    errorCount++;
    yyerror(("Function was not declared. Error at line: " + std::to_string(yylineno)).c_str());

    currentParams.clear();
    return false;
}


#endif