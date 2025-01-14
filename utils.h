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

//Variabile utilizate pentru asignari, adaugari in tabele de simboluir
FuncInfo currentFunction;
VarInfo currentVariable;
VarInfo *modifiedVariable;
VarSign variableToAssign;
VarSign variableFromExpr;

//pentru a putea prelua numele clasei curente
ClassInfo currentClass;

//variabile utilizate la definirea unei functii + apelul functiilor
vector<VarInfo> currentParams;
vector<VarInfo> currentCallList;
vector<short> currentArraySizes;

//class SymTable* current;
//tabelele de simboluri
vector<SymTable*> tables;
SymTable* globalTable = new SymTable("global");
SymTable* currentTable = globalTable;

// pentru a mentine returnType-ul unor functii, membri ai unor clase
short functionReturnType;
short objectMemberReturnType;

//numele clasei curente din care face parte un obiect
string currentClassName;

//pentru a avea nume de scope-uri diferite si imbricate
stack<string> scopeStack;
unordered_map<string, int> scopeCounters;

//aici vom stoca output-ul pe care il construim pe parcurs ce apelam functiile Print si TypeOf
string printToScreen;

using namespace std;

extern int yylineno; //numarul liniei curente
extern int errorCount; //numarul de erori

//seteaza o valoare default in dependenta de tipul de date pe care il are(int, float, ...) este apelat atunci cand declaram o variabila
void SetDefaultValue(VarInfo &var); 

//adaugam un scope nou cu un nume unic pentru for/while/if/loop
void addScopeName(const string& scopeType) 
{
    string ScopeName = scopeType;
    if (!scopeStack.empty()) 
    {
        ScopeName += to_string(scopeCounters[scopeType]++) + "_" + scopeStack.top();
    } 
    else 
    {
        ScopeName += to_string(scopeCounters[scopeType]++);
    }
    scopeStack.push(ScopeName);
    currentTable = new SymTable(ScopeName, currentTable);
    tables.push_back(currentTable);
}

//verificam daca o variabila utilizata sau care doreste a fi declarata se regaseste in lista de parametri curenti ai unei functii
bool existsParam(const string& name) 
{
    if(currentParams.size() == 0) //daca este 0, nu ne aflam in block-ul unei functii
    {
        return false;
    }
    for(auto param : currentParams) //iteram prin parametri pentru a vedea daca exista id-ul variabilei acolo
    {
        if(param.name == name) 
        {
            return true;
        }
    }
    return false;
}

//verificam daca o variabila exista in scope-ul curent si daca nu, o adaugam
bool exists_or_add(const string& name, bool is_array) 
{
    //cout << "Current table name: " << currentTable->ScopeName << endl;
    bool existsVarInParams = existsParam(name);
    if (currentTable->existsId(name) || existsVarInParams) 
    {
        errorCount++; 
        yyerror(("Variabila " + name + " a fost deja definită în cadrul acestui scope!").c_str());
        return false;
    } 
    
    if(is_array == true) 
    {
        currentVariable.type.isArray = true;
        currentVariable.type.arraySizes = currentArraySizes;
    } 
    else 
    {
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

//adaugam o instanta a unei clase in tabelul de simboluri in care ne aflam, doar daca nu exista una deja cu acest nume in tabelul curent
bool exists_or_add_for_custom_type(const string& objectName, const string& className) 
{
    if (currentTable->existsId(objectName)) 
    {
        errorCount++; 
        yyerror(("Variabilă deja definită în acest scop la linia: " + std::to_string(yylineno)).c_str());
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

//verificam parametrii unei functii sa fie aceiasi cu parametrii cu care am definit functia(nr de parametri, tipul acestora)
bool checkParams(const string& name, SymTable functionScope)
{
    int contor = 0;

    // Debug: Afișăm numărul de parametri așteptat și numărul de parametri actuali
    //std::cout << "Checking parameters for function: " << name << std::endl;
    //std::cout << "Expected params: " << functionScope.funcids[name].params.size() << ", Given params: " << currentCallList.size() << std::endl;

    if(functionScope.funcids[name].params.size() != currentCallList.size())
    {
        yyerror("Nu avem același număr de parametri! ");
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

//verificam apelul unei functii, daca este corect structurat si daca exista functia pe care dorim sa o apelam
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
                yyerror("Parametrii funcției nu au același tip! ");
                currentParams.clear();
                return false;
            }
        } 
    }

    // Dacă am ajuns aici, funcția nu a fost găsită în niciun tabel de simboluri
    errorCount++;
    yyerror("Funcția nu a fost declarată! ");

    currentParams.clear();
    return false;
}

//verificam o metoda dintr-o clasa, la fel ca si pentru functii, doar ca ne uitam doar in scope-ul clasei careia apartine obiectul
bool checkMethod(const string& methodName)
{
    int i;

    for(int i = 0; i < tables.size();i++) 
    {
        if(tables[i]->ScopeName == currentClassName) 
        {
            if(tables[i]->existsFunc(methodName)) 
            {
                if(checkParams(methodName, *tables[i])) 
                {
                    currentParams.clear();
                    return true;
                } 
                else 
                {
                    errorCount++;
                    yyerror("Parametrii funcției nu au același tip! ");
                    currentParams.clear();
                    return false;
                }
            } 
            else 
            {
                errorCount++;
                yyerror(("Metoda nu a fost declarată pentru clasa " + currentClassName + "! ").c_str());
                return false;
            }
        }
    }
   
    currentParams.clear();
    errorCount++;
    //std::cout << "" << std::endl;
    yyerror(("Clasa " + currentClassName + " nici măcar nu există! ").c_str());
    return false;
}

//ne uitam de ce tip este un obiect(clasa custom) si setam numele clasei in variabila currentClassName
bool setCurrentClassName(const string& objectName) 
{
    int i;
    for(i = 0; i < tables.size(); i++) 
    {
        if(tables[i]->existsId(objectName)) 
        {
            currentClassName = tables[i]->ids[objectName].type.className;
            //std::cout << "Set the currentClassName to " << tables[i]->ids[objectName].type.className;
            return true;
        }
    }

    return false;
}

//setam return type-ul pentru un membru al unei clase, verificand in tabela de simboluri a clase ce tip de date este
bool setObjectMemberReturnType(const string& objectName, const string& memberName) 
{
    setCurrentClassName(objectName);
    for(int i = 0; i < tables.size(); i++) 
    {
        if(tables[i]->ScopeName == currentClassName) 
        {
            objectMemberReturnType = tables[i]->ids[memberName].type.typeName;
            return true;
        }
    }
    return false;
}

//setam tipul de return al variabilei pe care dorim sa o utilizam
bool setCurrentVariableType(const string& varName) 
{

    SymTable* temp = currentTable;
    while(temp != nullptr) 
    {
        if (temp->existsId(varName)) 
        {
            currentVariable.type.typeName = temp->ids[varName].type.typeName;
            return true;
        } 
        temp = temp->prev;
    }

    return false;
}

//asingam tipul de date dorit pentru un numar(practic casting de la int la string)
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

//verificam daca membrul memberName este definit pentru clasa din care face parte objectName
bool checkObject(const string& objectName, const string& memberName) 
{
    if(!setCurrentClassName(objectName)) 
    {
        errorCount++;
        yyerror(("Instanța " + objectName + " nu a fost declarată înainte! " + std::to_string(yylineno)).c_str());
        return false;
    }
    bool memberFound = false;
    for(int i = 0; i < tables.size(); i++) 
    {
        if(tables[i]->ScopeName == currentClassName) 
        {
            for(const auto& [name, var] : tables[i]->ids) 
            {
                if(name == memberName) 
                {
                    memberFound = true;
                    break;
                }
            }
        }
    }
    if(!memberFound) 
    {
        errorCount++;
        yyerror(("Obiectul " + objectName + " nu are membrul " + memberName + "! " + std::to_string(yylineno)).c_str());
        return false;
    }
    return true;
}

//Setam variabilei var, valoarea value
void SetNewValue(VarInfo *var, ASTNode* value) 
{
    int type = value->GetType();

    if(var->type.typeName != type) 
    {
        yyerror(("Variabila " + var->name + " - partea dreaptă și cea stângă nu au același tip: " + getReturnType(var->type.typeName) + " - " + getReturnType(type) + "! ").c_str());
        errorCount++;
    }
    
    switch(type) 
    {
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
            yyerror("Nu putem asigna o expresie unei clase! ");
            errorCount++;
            break;
        default:
            if(type >= 6 || type < 0) {
                yyerror("Tip de date necunoscut! ");
                errorCount++;
                return;
            }
    }
    currentArraySizes.clear();
    variableToAssign = VarSign();
}

//setam valoare default pentur un array, maxim 2 dimensiuni
void SetArrayDefaultValue(VarInfo& var, size_t level = 0) 
{
    if (level >= var.type.arraySizes.size()) return;

    int size = var.type.arraySizes[level];
    for (int i = 0; i < size; i++) 
    {
        VarInfo element;
        element.type.typeName = var.type.typeName;
        element.type.isArray = (level + 1 < var.type.arraySizes.size());
        element.type.arraySizes = vector<short>(var.type.arraySizes.begin() + level, var.type.arraySizes.end());

        if (element.type.isArray) 
        {
            SetArrayDefaultValue(element, level + 1);
        } 
        else 
        {
            SetDefaultValue(element);
        }

        var.fields.push_back(element);
    }
}

//setam valoarea default pentru o variabila de orice tip(clasa, array, tip de date norma)
void SetDefaultValue(VarInfo &var) 
{
    int type = var.type.typeName;
    if(var.type.isArray == 1) 
    {
        SetArrayDefaultValue(var);
        return;
    }
    float value = 0.0;
    switch(type) 
    {
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
            for(auto &table : tables) 
            {
                if(table->ScopeName == var.type.className) 
                {
                    //cout << "Current class name: " << table->ScopeName << endl;
                    for(auto &[name, field] : table->ids) 
                    {
                        //SetDefaultValue(field);
                        var.fields.push_back(VarInfo(field));
                        if(field.type.isArray == 1) 
                        {
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

//introducem in varful stivei o operatie binara, construind-o din cele 2 noduri din varful stivei si operatia binara op
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

//cautam variabila care trebuie modificata prin tabelele de simboluri la care avem acess din tabelul curent(doar in prev, prev->prev ...)
bool FindToBeModifiedVar(VarSign variable) 
{
    bool varExistsInParams = existsParam(variable.varName);
    SymTable* temp = currentTable;
    while(temp != nullptr) 
    {
        if (temp->existsId(variable.varName)) 
        {
            break;
        } 
        temp = temp->prev;
    }

    if(temp == nullptr && !varExistsInParams) 
    {
        yyerror(("Variabila " + variable.varName + " nu a fost declarată! " ).c_str());
        errorCount++;
        return false;
    }

    if(varExistsInParams) 
    {
        //cout << "Am gasit variabila in parametri" << endl;
        //cout << currentParams.size() << endl;
        for(auto &param : currentParams) 
        {
            if(variable.varName == param.name) 
            {
                //cout << "Am gasit variabila " << param.name << endl;
                modifiedVariable = &param;
                return true;
            }
        }
        return false;
    }

    // 0 tip simplu, 1 element al unui array, 2 membru al unei clase(a unui obiect)

    if(variable.varType == 0) 
    {
        modifiedVariable = &temp->ids[variable.varName];
        return true;
    }

    if(variable.varType == 1) 
    {
        if(!temp->ids[variable.varName].type.isArray)
         {
            yyerror(("Variabla nu este de tip array! \t " + std::to_string(yylineno) + "\n").c_str());
            errorCount++;
            return false;
        }

        if(variable.varIndex[0] >= temp->ids[variable.varName].type.arraySizes[0] || variable.varIndex[0] < 0) 
        {
            yyerror(("Indexul este în afara limitelor! \t " + std::to_string(yylineno) + "\n").c_str());
            errorCount++;
            return false;
        }
        modifiedVariable = &temp->ids[variable.varName].fields[variable.varIndex[0]];
        return true;
    }

    if(variable.varType == 2) 
    {
    // Parcurgem câmpurile clasei pentru a găsi membrul necesar
        for(int j = 0; j < temp->ids[variable.varName].fields.size(); j++) 
        {
            if(variable.varField == temp->ids[variable.varName].fields[j].name) 
            {
                VarInfo* field = &temp->ids[variable.varName].fields[j];

                // Verificăm dacă membrul este un array
                if(field->type.isArray && variable.varIndex[0] != 0) 
                {
                    // Verificăm indexul și dimensiunea array-ului
                    if(variable.varIndex[0] >= field->type.arraySizes[0] || variable.varIndex[0] < 0) 
                    {
                        yyerror(("Index în afara limitelor pentru membrul array! \t " + std::to_string(yylineno) + "\n").c_str());
                        errorCount++;
                        return false;
                    }

                    // Accesăm elementul specificat de variable.varIndex[0]
                    modifiedVariable = &field->fields[variable.varIndex[0]];
                    return true;
                } 
                else 
                {
                    // Dacă nu este array, returnăm direct membrul
                    modifiedVariable = field;
                    return true;
                }
            }
        }
    }

    yyerror(("Variabila nedefinită in interiorul clasei!\t " + std::to_string(yylineno) + "\n").c_str());
    errorCount++;
    return false;

}

//setam valori array-ului prin lista de initializare
void UpdateArray(VarInfo *var) 
{
    if(ArrayInitialization.size() > var->type.arraySizes[0]) 
    {
        yyerror("Prea multe elemente in lista de inițializare! ");
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

    for(int i = 0; i < ArrayInitialization.size(); i++) 
    {
        SetNewValue(&v, ArrayInitialization[i]);
        var->fields.push_back(v);
    }

    SetDefaultValue(v);

    for(i; i < var->type.arraySizes[0]; i++)
    {
        var->fields.push_back(v);
    }
    ArrayInitialization.clear();
}

//punem o variabila pe stiva
void PushVariableToStack() 
{
     
     if(!FindToBeModifiedVar(variableFromExpr)) 
     {
          stiva.push_back(new ASTNode(0));
          return;
     }
     
     switch(modifiedVariable->type.typeName) 
     {
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

//rulam functia print
void runPrint() 
{
    expr1 = stiva.back();
    stiva.pop_back();

    int type = expr1->GetType();
    char buffer[64];
    sprintf(buffer, "Linia: %d\t", yylineno); 

    printToScreen += buffer;
    switch(type) 
    {
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

//rulam functia TypeOf
void runTypeOf() 
{
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