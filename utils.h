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
short functionReturnType;
short objectMemberReturnType;
string currentClassName;

using namespace std;

extern int yylineno;
extern int errorCount;


bool exists_or_add(const string& name, bool is_array) {

    //cout << "Current table name: " << currentTable->ScopeName << endl;
    if (currentTable->existsId(name)) {
        errorCount++; 
        yyerror(("Variable already defined in this scope at line: " + std::to_string(yylineno)).c_str());
        return false;
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

bool exists_or_add_2(const string& name, bool is_array) {
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
        currentParams.clear();
        return false;
    }

    // Comparăm tipurile parametrilor
    for(auto param : functionScope.funcids[name].params)
    {
        //pentru debug
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
            //pentru debug
            std::cout << "Function not found in current scope. Checking previous scope..." << std::endl;
            temp = temp->prev; 
            continue;
        }
        else
        {
            //pentru debug
            std::cout << "Function found in current scope." << std::endl;

            //salvam return type-ul functiei!
            functionReturnType = temp->funcids[name].returnType;
            // Verificăm parametrii funcției
            if(checkParams(name, *temp))
            {
                //debug
                std::cout << "Function and parameters match." << std::endl;
                currentParams.clear();
                return true;
            }
            else
            {
                errorCount++;
                yyerror(("Function's params are not the same type. Error at line: " + std::to_string(yylineno)).c_str());
                currentParams.clear();
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

bool checkMethod(const string& methodName)
{
    int i;

    for(int i = 0; i < tables.size();i++) {
        if(tables[i]->ScopeName == currentClassName) {
            if(tables[i]->existsFunc(methodName)) {
                if(checkParams(methodName, *tables[i])) {
                    currentParams.clear();
                    return true;
                } else {
                    errorCount++;
                    yyerror(("Function's params are not the same type. Error at line: " + std::to_string(yylineno)).c_str());
                    currentParams.clear();
                    return false;
                }
            } else {
                errorCount++;
                yyerror(("Method not declared for class " + currentClassName + " . Error at line: " + std::to_string(yylineno)).c_str());
            }
        }
    }
   
    currentParams.clear();
    errorCount++;
    //std::cout << "" << std::endl;
    yyerror(("Class " + currentClassName + " does not exist . Error at line: " + std::to_string(yylineno)).c_str());
    return false;
}

bool setCurrentClassName(const string& objectName) {
    int i;
    for(i = 0; i < tables.size(); i++) {
        if(tables[i]->existsId(objectName)) {
            currentClassName = tables[i]->ids[objectName].type.className;
            //std::cout << "Set the currentClassName to " << tables[i]->ids[objectName].type.className;
            return true;
        }
    }

    return false;
}

bool setObjectMemberReturnType(const string& objectName, const string& memberName) {
    setCurrentClassName(objectName);
    for(int i = 0; i < tables.size(); i++) {
        if(tables[i]->ScopeName == currentClassName) {
            objectMemberReturnType = tables[i]->ids[memberName].type.typeName;
            return true;
        }
    }
    return false;
}

bool setCurrentVariableType(const string& varName) {
    SymTable* temp = currentTable;
    while(temp != nullptr) {
        if (temp->existsId(varName)) {
            currentVariable.type.typeName = temp->ids[varName].type.typeName;
            return true;
        } 
        temp = temp->prev;
    }

    return false;
}


#endif