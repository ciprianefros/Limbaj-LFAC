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
vector<VarInfo> currentCallList;
vector<short> currentArraySizes;
SymTable* current_p;

//class SymTable* current;
vector<SymTable*> tables;
SymTable* globalTable = new SymTable("global");
SymTable* currentTable = globalTable;
SymTable* p;
short functionReturnType;
short objectMemberReturnType;
string currentClassName;

stack<string> scopeStack;
unordered_map<string, int> scopeCounters;
string printToScreen;

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

bool existsParam(const string& name) {
    if(currentParams.size() == 0) {
        return false;
    }
    for(auto param : currentParams) {
        if(param.name == name) {
            return true;
        }
    }
    return false;
}

bool exists_or_add(const string& name, bool is_array) {

    //cout << "Current table name: " << currentTable->ScopeName << endl;
    bool existsVarInParams = existsParam(name);
    if (currentTable->existsId(name) || existsVarInParams) {
        errorCount++; 
        yyerror(("Variable " + name + " already defined in this scope").c_str());
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

bool checkParams(const string& name, SymTable functionScope)
{
    int contor = 0;

    // Debug: Afișăm numărul de parametri așteptat și numărul de parametri actuali
    //std::cout << "Checking parameters for function: " << name << std::endl;
    //std::cout << "Expected params: " << functionScope.funcids[name].params.size() << ", Given params: " << currentCallList.size() << std::endl;

    if(functionScope.funcids[name].params.size() != currentCallList.size())
    {
        yyerror("Not having the same number of params.");
        currentParams.clear();
        return false;
    }

    // Comparăm tipurile parametrilor
    for(auto param : functionScope.funcids[name].params)
    {
        //pentru debug
        //std::cout << "Expected param type: " << param.type.typeName << ", Given param type: " << currentParams[contor].type.typeName << std::endl;
        if(param.type.typeName != currentCallList[contor].type.typeName)
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
            //cout << "Tipul functiei: " << functionReturnType << endl;
            // Verificăm parametrii funcției
            if(checkParams(name, *temp))
            {
                //debug
                //std::cout << "Function and parameters match." << std::endl;
                //currentParams.clear();
                return true;
            }
            else
            {
                errorCount++;
                yyerror("Function's params are not the same type. ");
                currentParams.clear();
                return false;
            }
        } 
    }

    // Dacă am ajuns aici, funcția nu a fost găsită în niciun tabel de simboluri
    errorCount++;
    yyerror("Function was not declared. ");

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
                    yyerror("Function's params are not the same type. ");
                    currentParams.clear();
                    return false;
                }
            } else {
                errorCount++;
                yyerror(("Method not declared for class " + currentClassName + ". ").c_str());
                return false;
            }
        }
    }
   
    currentParams.clear();
    errorCount++;
    //std::cout << "" << std::endl;
    yyerror(("Class " + currentClassName + " does not exist. ").c_str());
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
        printf("error: Variable: %s - right and left are not the same type %s - %s at line %d\n", var->name.c_str(),getReturnType(var->type.typeName).c_str(), getReturnType(type).c_str(), yylineno);
        errorCount++;
    }

    
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
    currentArraySizes.clear();
    variableToAssign = VarSign();
}

void SetArrayDefaultValue(VarInfo& var, size_t level = 0) {
    if (level >= var.type.arraySizes.size()) return;

    int size = var.type.arraySizes[level];
    for (int i = 0; i < size; i++) {
        VarInfo element;
        element.type.typeName = var.type.typeName;
        element.type.isArray = (level + 1 < var.type.arraySizes.size());
        element.type.arraySizes = vector<short>(var.type.arraySizes.begin() + level, var.type.arraySizes.end());

        if (element.type.isArray) {
            SetArrayDefaultValue(element, level + 1);
        } else {
            SetDefaultValue(element);
        }

        var.fields.push_back(element);
    }
}

void SetDefaultValue(VarInfo &var) {
    int type = var.type.typeName;
    if(var.type.isArray == 1) {
        SetArrayDefaultValue(var);
        return;
    }
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
                        if(field.type.isArray == 1) {
                            SetArrayDefaultValue(field);
                        }
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
    bool varExistsInParams = existsParam(variable.varName);
    SymTable* temp = currentTable;
    while(temp != nullptr) {
        if (temp->existsId(variable.varName)) {
            break;
        } 
        temp = temp->prev;
    }
    if(temp == nullptr && !varExistsInParams) {
        yyerror(("Variabila " + variable.varName + " nu a fost declarata " ).c_str());
        errorCount++;
        return false;
    }
    if(varExistsInParams) {
        //cout << "Am gasit variabila in parametri" << endl;
        //cout << currentParams.size() << endl;
        for(auto &param : currentParams) {
            if(variable.varName == param.name) {
                //cout << "Am gasit variabila " << param.name << endl;
                modifiedVariable = &param;
                return true;
            }
        }
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
    // Parcurgem câmpurile clasei pentru a găsi membrul necesar
        for(int j = 0; j < temp->ids[variable.varName].fields.size(); j++) {
            if(variable.varField == temp->ids[variable.varName].fields[j].name) {
                VarInfo* field = &temp->ids[variable.varName].fields[j];

                // Verificăm dacă membrul este un array
                if(field->type.isArray && variable.varIndex[0] != 0) {
                    // Verificăm indexul și dimensiunea array-ului
                    if(variable.varIndex[0] >= field->type.arraySizes[0] || variable.varIndex[0] < 0) {
                        yyerror(("Index out of bounds pentru membrul array\t ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
                        errorCount++;
                        return false;
                    }

                    // Accesăm elementul specificat de variable.varIndex[0]
                    modifiedVariable = &field->fields[variable.varIndex[0]];
                    return true;
                } else {
                    // Dacă nu este array, returnăm direct membrul
                    modifiedVariable = field;
                    return true;
                }
            }
        }
    }

    yyerror(("Variabila nedefinită in interiorul clasei\t ERROR LINE: " + std::to_string(yylineno) + "\n").c_str());
    errorCount++;
    return false;

}

void UpdateArray(VarInfo *var) {
    if(ArrayInitialization.size() > var->type.arraySizes[0]) {
        yyerror("Prea multe elemente in lista de initializare!");
        errorCount++;
        SetArrayDefaultValue(*var);
        ArrayInitialization.clear();
        return;
    }
    //cout << "Marimea array-ului: " << var->type.arraySizes[0] << endl;
    //cout << "Numele array-ului:" << var->name << endl;
    VarInfo v;
    v.type.typeName = var->type.typeName;
    v.type.isArray = false;
    int i = 0;
    var->fields.clear();
    for(int i = 0; i < ArrayInitialization.size(); i++) {
        SetNewValue(&v, ArrayInitialization[i]);
        var->fields.push_back(v);
    }
    SetDefaultValue(v);
    for(i; i < var->type.arraySizes[0]; i++) {
        var->fields.push_back(v);
    }
    ArrayInitialization.clear();
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

void runPrint() {
    expr1 = stiva.back();
    stiva.pop_back();

    int type = expr1->GetType();
    char buffer[64];
    sprintf(buffer, "Linia: %d\t", yylineno); 

    printToScreen += buffer;
    switch(type) {
        case TYPE_BOOL:
            sprintf(buffer, "%s\n", expr1->GetBoolValue() ? "true" : "false");
            printToScreen += buffer;
            break;
        case TYPE_INT:
            sprintf(buffer, "%d\n", expr1->GetIntValue());
            printToScreen += buffer;
            break;
        case TYPE_FLOAT:
            sprintf(buffer, "%f\n", expr1->GetFloatValue());
            printToScreen += buffer;
            break;
        case TYPE_CHAR:
            sprintf(buffer, "%c\n", expr1->GetCharValue());
            printToScreen += buffer;
            break;
        case TYPE_STRING:
            sprintf(buffer, "%s\n", expr1->GetStringValue());
            printToScreen += buffer;
            break;
    }
}

void runTypeOf() {
     expr1 = stiva.back();
     stiva.pop_back();

     int type = expr1->GetType();
     char buffer[64];
     sprintf(buffer, "Linia: %d\t", yylineno);
     printToScreen += buffer;
     switch(type) {
          case TYPE_BOOL:
               printToScreen += "bool\n";
               break;
          case TYPE_INT:
               printToScreen += "int\n";
               break;
          case TYPE_FLOAT:
               printToScreen += "float\n";
               break;
          case TYPE_CHAR:
               printToScreen += "char\n";
               break;
          case TYPE_STRING:
               printToScreen += "string\n";
               break;
          default: 
               sprintf(buffer, "%s\n", currentClassName.c_str());
               printToScreen += buffer;
     }
}

#endif