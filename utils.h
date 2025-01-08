#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "SymTable.h"
#include "AST.h"

void yyerror(const char * s);

FuncInfo currentFunction;
VarInfo currentVariable;
VarInfo *modifiedVariable;
VarSign variableToAssign;
VarSign variableFromExpr;
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

stack<string> scopeStack;
unordered_map<string, int> scopeCounters;

using namespace std;

extern int yylineno;
extern int errorCount;

void SetDefaultValue(VarInfo &var); 

void addScopeName(const string& scopeType) {
    string ScopeName = scopeType;
    if (!scopeStack.empty()) {
        ScopeName += to_string(scopeCounters[scopeType]++) + "_" + scopeStack.top();
    } else {
        ScopeName += to_string(scopeCounters[scopeType]++);
    }
    scopeStack.push(ScopeName);
    currentTable = new SymTable(ScopeName, currentTable);
    tables.push_back(currentTable);
}

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
        for(int i = 0; i < currentArraySizes[0]; i++) {
            currentVariable.fields.push_back(VarInfo());
        }
    } else {
        currentVariable.type.isArray = false;
    }
    currentVariable.name = name;
    currentVariable.value.setType(currentVariable.type.typeName);
    
    SetDefaultValue(currentVariable);
    currentTable->addVar(currentVariable);
    currentArraySizes.clear();
    currentVariable = VarInfo();
    return true;
}
bool exists_or_add_for_custom_type(const string& objectName, const string& className) {
    if (currentTable->existsId(objectName)) {
        errorCount++; 
        yyerror(("Variable already defined in this scope at line: " + std::to_string(yylineno)).c_str());
        return false;
    } 
    
    currentVariable.type.isArray = false;
    currentVariable.name = objectName;
    currentVariable.type.typeName = 5;
    currentVariable.value.setType(currentVariable.type.typeName);
    currentVariable.type.className = className;
    
    SetDefaultValue(currentVariable);
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
    //std::cout << "Checking parameters for function: " << name << std::endl;
    //std::cout << "Expected params: " << functionScope.funcids[name].params.size() << ", Given params: " << currentParams.size() << std::endl;

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
        //std::cout << "Expected param type: " << param.type.typeName << ", Given param type: " << currentParams[contor].type.typeName << std::endl;
        if(param.type.typeName != currentParams[contor].type.typeName)
        {
            //std::cout << "Type mismatch at parameter " << contor + 1 << std::endl;
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
    //std::cout << "Checking function: " << name << std::endl;

    while(temp != nullptr) 
    {
        // Verificăm dacă funcția există în tabelul curent
        if (!temp->existsFunc(name)) 
        {
            //pentru debug
            //std::cout << "Function not found in current scope. Checking previous scope..." << std::endl;
            temp = temp->prev; 
            continue;
        }
        else
        {
            //pentru debug
            //std::cout << "Function found in current scope." << std::endl;

            //salvam return type-ul functiei!
            functionReturnType = temp->funcids[name].returnType;
            // Verificăm parametrii funcției
            if(checkParams(name, *temp))
            {
                //debug
                //std::cout << "Function and parameters match." << std::endl;
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
                return false;
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

string getReturnType(int returnType)
{
     if(returnType == 0)
     {
          return "int";
     }
     else if(returnType == 1)
     {
          return "float";
     }
     else if(returnType == 2)
     {
          return "char";
     }
     else if(returnType == 3)
     {
          return "bool";
     }
     else if(returnType == 4)
     {
          return "string";
     }
     else if(returnType == 5)
     {
        return "Custom Class";
     }

     return "";     
}

bool checkObject(const string& objectName, const string& memberName) {
    if(!setCurrentClassName(objectName)) {
        errorCount++;
        yyerror(("Object " + objectName + " was not previously declared." + std::to_string(yylineno)).c_str());
        return false;
    }
    bool memberFound = false;
    for(int i = 0; i < tables.size(); i++) {
        if(tables[i]->ScopeName == currentClassName) {
            for(const auto& [name, var] : tables[i]->ids) {
                if(name == memberName) {
                    memberFound = true;
                    break;
                }
            }
        }
    }
    if(!memberFound) {
        errorCount++;
        yyerror(("Object " + objectName + " does not have member" + memberName + ". Error " + std::to_string(yylineno)).c_str());
        return false;
    }
    return true;
}

void SetNewValue(VarInfo *var, ASTNode* value) {
     int type = value->GetType();

     if(var->type.typeName != type) {
          printf("ERROR LINE:%d - %s right and left are not the same type %s - %s\n", yylineno, var->name.c_str(),getReturnType(var->type.typeName).c_str(), getReturnType(type).c_str());
          errorCount++;
     }

     // de verificat sa nu fie const dar doar in instructiuni nu si la initializare
     switch(type) {
          case TYPE_BOOL:
               var->value = Value(value->GetBoolValue());
               break;
          case TYPE_INT:
               var->value = Value(value->GetIntValue());
               break;
          case TYPE_FLOAT:
               var->value = Value(value->GetFloatValue());
               break;
          case TYPE_CHAR:
               var->value = Value(value->GetCharValue());
               break;
          case TYPE_STRING:
               var->value = Value(value->GetStringValue());
               break;
          case CUSTOM_TYPE:
               printf("Cannot assign expr to a class");
               errorCount++;
               break;
          default:
               if(type >= 6 || type < 0) {
                    printf("EROARE LINE:%d\tUnknown type\n", yylineno);
                    errorCount++;
                    return;
               }
     }
}

void SetDefaultValue(VarInfo &var) {
     int type = var.type.typeName;
     float value = 0.0;
     switch(type) {
            case TYPE_INT:
                var.value = Value(0);
                break;
            case TYPE_FLOAT:
                var.value = Value(value);
                break;
            case TYPE_CHAR:
                var.value = Value('0');
                break;
            case TYPE_STRING:
                var.value = Value(strdup("0"));
                break;
            case TYPE_BOOL:
                var.value = Value(false);
                break;
            case CUSTOM_TYPE:
                for(auto &table : tables) {
                    if(table->ScopeName == var.type.className) {
                        //cout << "Current class name: " << table->ScopeName << endl;
                        for(auto &[name, field] : table->ids) {
                            SetDefaultValue(field);
                            var.fields.push_back(VarInfo(field));
                        }
                    }
                }
                break;
          default: var.value = Value(0);
               
     }
}

/*ASTNode*/

vector<ASTNode*> ArrayInitialization;
vector<ASTNode*> stiva;
ASTNode *expr1, *expr2; 

void Operation_on_stack(B_operation op) 
{
    //retinem si eliminam primul nod
     expr2 = stiva.back(); 
     stiva.pop_back();

    //retinem si eliminam al doilea nod
     expr1 = stiva.back(); 
     stiva.pop_back(); 

    // adaugam operatia pe stiva
     stiva.push_back(new ASTNode(op, expr1, expr2));
}

bool FindToBeModifiedVar(VarSign variable) {
    SymTable* temp = currentTable;
    while(temp != nullptr) {
        if (temp->existsId(variable.varName)) {
            break;
        } 
        temp = temp->prev;
    }
    if(temp == nullptr) {
        yyerror(("Variabila " + variable.varName + " nu a fost declarata ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
        errorCount++;
        return false;
    }

    if(variable.varType == 0) {
        modifiedVariable = &temp->ids[variable.varName];
        return true;
    }

    if(variable.varType == 1) {
        if(!temp->ids[variable.varName].type.isArray) {
            yyerror(("Variabla nu este de tip array\t ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
            errorCount++;
            return false;
        }

        if(variable.varIndex[0] >= temp->ids[variable.varName].type.arraySizes[0] || variable.varIndex[0] < 0) {
            yyerror(("Index is out of bounds\t ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
            errorCount++;
            return false;
        }

        modifiedVariable = &temp->ids[variable.varName].fields[variable.varIndex[0]];
        return true;
    }

    if(variable.varType == 2) {
        //printf("Size of fields for class: %ld", temp->ids[variable.varName].fields.size());
        for(int j = 0; j < temp->ids[variable.varName].fields.size(); j++) {
            if(variable.varField == temp->ids[variable.varName].fields[j].name) {
                modifiedVariable = &temp->ids[variable.varName].fields[j];
                return true;
            }
        }
    }

    yyerror(("Variabila nedefinită in interiorul clasei\t ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
    errorCount++;
    return false;

}
void PushVariableToStack() {
     
     if(!FindToBeModifiedVar(variableFromExpr)) {
          stiva.push_back(new ASTNode(0));
          return;
     }
     
     switch(modifiedVariable->type.typeName) {
          case TYPE_INT:
               stiva.push_back(new ASTNode(modifiedVariable->value.getIntValue()));
               break;
          case TYPE_FLOAT:
               stiva.push_back(new ASTNode(modifiedVariable->value.getFloatValue()));
               break;
          case TYPE_CHAR:
               stiva.push_back(new ASTNode(modifiedVariable->value.getCharValue()));
               break;
          case TYPE_STRING:
               stiva.push_back(new ASTNode(modifiedVariable->value.getStringValue()));
               break;
          case TYPE_BOOL:
               stiva.push_back(new ASTNode(modifiedVariable->value.getBoolValue()));
               break;
          default: 
               stiva.push_back(new ASTNode(0, modifiedVariable->type.typeName));
     }
}

#endif