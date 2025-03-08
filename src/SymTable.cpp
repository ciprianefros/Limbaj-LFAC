#include "SymTable.h"
#include <string.h>

// Definiția clasei Value
//Constructori
Value::Value() : type(-1) {}

Value::Value(int x) : intValue(x), type(0) {}

Value::Value(float x) : floatValue(x), type(1) {}

Value::Value(char x) : charValue(x), type(2) {}

Value::Value(bool x) : boolValue(x), type(3) {}

Value::Value(char* x) : stringValue(x), type(4) {}

//returnam in format string valoarea
string Value::toString() 
{
    if (type == 0) return to_string(intValue);
    if (type == 1) return to_string(floatValue);
    if (type == 2) 
    {
        string str(1, charValue);
        return str;
    };
    if (type == 3) 
    {
        if(boolValue == true) 
        {
            return "true";
        } 
        else 
        {
            return "false";
        }
    }
    if (type == 4) return stringValue;
    return "Custom Type";
}

int Value::getIntValue() 
{
    if(type == 0) 
    {
        return intValue;
    }
    return 0;
}

float Value::getFloatValue()
{
    if(type == 1) 
    {
        return floatValue;
    }
    return 0.0;
}

char Value::getCharValue() 
{
    if(type == 2) 
    {
        return charValue;
    }
    return '\0';
}

bool Value::getBoolValue() 
{
    if(type == 3) 
    {
        return boolValue;
    }
    return NULL;
}

char* Value::getStringValue() 
{
    if(type == 4) 
    {
        return stringValue;
    }
    return NULL;
}

int Value::getType() const 
{
    return type;
}
void Value::setType(short type) 
{
    this->type = type;
}

// Definiția clasei VarInfo
VarInfo::VarInfo() : value(), name(""), type(), fields() {}

VarInfo::VarInfo(short type, const string& name)
{
    this->type.typeName = type;
    this->name = name;
}

VarInfo::VarInfo(short type, const string& name, Value valoare)
{
    this->type.typeName = type;
    this->name = name;
    this->value = valoare;
}

VarInfo::VarInfo(const VarInfo& v)
{
    this->type = v.type;
    this->type.className = v.type.className;
    this->name = v.name;
    this->value = v.value;
    this->fields = v.fields;
}

// Definiția clasei FuncInfo
FuncInfo::FuncInfo() {}

FuncInfo::FuncInfo(short type, const string& name, vector<VarInfo> params)
{
    this->returnType = type;
    this->name = name;
    this->params = params;
}

FuncInfo::FuncInfo(short type, const string& name, vector<VarInfo> params, const string& className)
{
    this->returnType = type;
    this->name = name;
    this->params = params;
    this->className = className;
}


//Definitia clasei ClassInfo
ClassInfo::ClassInfo(const string& name) : name(name) {}

ClassInfo::~ClassInfo() 
{
    // Curățare sau alte operațiuni dacă este necesar
}

// Definiția clasei SymTable
SymTable::SymTable(const string& name, SymTable* parent)
    : ScopeName(name), prev(parent) {}



bool SymTable::existsId(const string& name) 
{
    if(ids.find(name) != ids.end()) 
    {
        return true;
    }
    return false;
}

bool SymTable::existsFunc(const string& name) 
{
    if(funcids.find(name) != funcids.end()) 
    {
        return true;
    }
    return prev ? prev->existsId(name) : false;
}

bool SymTable::existsClass(const string& name) 
{
    if(classids.find(name) != classids.end()) 
    {
        return true;
    }
    return prev ? prev->existsId(name) : false;
}

bool SymTable::addVar(short type, const string& name) 
{
    if(ids.find(name) != ids.end()) 
    {
        return false;
    }
    ids[name] = VarInfo(type, name);
    return true;
}

bool SymTable::addVar(short type, const string& name, Value valoare) 
{
    if(ids.find(name) != ids.end()) 
    {
        return false;
    }
    if(type == valoare.getType()) 
    {
        ids[name] = VarInfo(type, name, valoare);
        return true;
    } 
    else 
    {
        printf("ERROR LINE:%d\tTipul variabilei nu coincide cu tipul de date asignat!\n", yylineno);
        return false;
    }
    
}

bool SymTable::addVar(const VarInfo& v) 
{
    if(ids.find(v.name) != ids.end()) 
    {
        return false;
    }
    if(v.type.typeName == v.value.getType()) 
    {
        ids[v.name] = VarInfo(v);
        return true;
    } 
    else 
    {
        //printf("TypeName = %d, Value.getType() = %d", v.type.typeName, v.value.getType());
        printf("ERROR LINE:%d\tTipul variabilei nu coincide cu tipul de date asignat!\n", yylineno);
        return false;
    }
    
}

bool SymTable::addFunc(short type, const string& name, vector<VarInfo> params) 
{
    if(funcids.find(name) != funcids.end()) 
    {
        return false;
    }
    funcids[name] = FuncInfo(type, name, params);
    return true;
}

bool SymTable::addFunc(short type, const string& name, vector<VarInfo> params, const string& className) 
{
    if(funcids.find(name) != funcids.end()) 
    {
        return false;
    }
    funcids[name] = FuncInfo(type, name, params, className);
    return true;
}

bool SymTable::addClass(const string& name) 
{
    if(classids.find(name) != classids.end()) 
    {
        return false;
    }
    classids[name] = ClassInfo(name);
    return true;
}
//printam un array
void SymTable::printArrayValues(VarInfo& var, std::ofstream& outFile, size_t level = 0)
 {
    if (level >= var.type.arraySizes.size()) 
    {
        outFile << var.value.toString(); // Afișăm valoarea elementului final
        return;
    }

    outFile << "{";
    for (size_t i = 0; i < var.type.arraySizes[level]; i++) {
        if (i > 0) outFile << ", ";
        printArrayValues(var.fields[i], outFile, level + 1);
    }
    outFile << "}";
}

//punem in scope.txt tabelele de simboluri
void SymTable::printTable(const string& filename) 
{
    std::ofstream outFile(filename, ios::app); // Deschide fișierul în modul append

    if (!outFile) 
    {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    outFile << "Scope: " << ScopeName << "\n";
    outFile << "Variabile:\n";
    for (auto& [name, var] : ids) 
    {
        string type;

        switch(var.type.typeName) 
        {
            case 0 :    {type = "intreg"; break;}
            case 1 :    {type = "real"; break;}
            case 2 :    {type = "caracter"; break;}
            case 3 :    {type = "bool"; break;}
            case 4 :    {type = "sir"; break;}
            case 5 :    {type = var.type.className; break;}
            default :   {type = "customType";}
        }

        if (!var.type.isArray) 
        {
            if(var.type.typeName == 5) 
            {
                outFile << "  " << type << " " << var.name << " {" << endl;
                for(auto field : var.fields) 
                {
                    string membType;
                    switch(field.type.typeName) 
                    {
                        case 0 :    {membType = "intreg"; break;}
                        case 1 :    {membType = "real"; break;}
                        case 2 :    {membType = "caracter"; break;}
                        case 3 :    {membType = "bool"; break;}
                        case 4 :    {membType = "sir"; break;}
                        case 5 :    {membType = var.type.className; break;}
                        default :   {membType = "customType";}
                    }
                    if(!field.type.isArray) 
                    {
                        outFile << "\t\t" << membType << " " << field.name << " = " << field.value.toString() << ";\n";
                    } 
                    else 
                    {
                        outFile << "\t\t" << membType << " " << field.name << "[";
                        for (size_t i = 0; i < field.type.arraySizes.size(); i++) 
                        {
                            if (i > 0) outFile << ",";
                            outFile << field.type.arraySizes[i];
                        }
                        outFile << "] = ";
                        printArrayValues(field, outFile);
                        outFile << "\n";
                    }
                }
                outFile << "\t}\n";
            } 
            else 
            {
                outFile << "  " << type << " " << var.name << " = " << var.value.toString() << "\n";
            }
        } 
        else 
        {
            outFile << "  " << type << " " << var.name << "[";
            for (size_t i = 0; i < var.type.arraySizes.size(); i++) 
            {
                if (i > 0) outFile << ",";
                outFile << var.type.arraySizes[i];
            }
            outFile << "] = ";
            printArrayValues(var, outFile);
            outFile << "\n";
        }
    }

    outFile << "Functions:\n";
    for (const auto& [name, func] : funcids)
    {
        string type1;
        switch(func.returnType) 
        {
            case 0 : {type1 = "intreg"; break;}
            case 1 : {type1 = "real"; break;}
            case 2 : {type1 = "caracter"; break;}
            case 3 : {type1 = "bool"; break;}
            case 4 : {type1 = "sir"; break;}
            default : {type1 = "customType";}
        }
        outFile << "  " << type1 << " " << func.name << " (";

        for(auto param : func.params) 
        {
            string type2;
            switch(param.type.typeName) 
            {
                case 0 : {type2 = "intreg"; break;}
                case 1 : {type2 = "real"; break;}
                case 2 : {type2 = "caracter"; break;}
                case 3 : {type2 = "bool"; break;}
                case 4 : {type2 = "sir"; break;}
                default : {type2 = "customType";}
            }
            outFile << type2 << " " << param.name;
            outFile << ",";
        }
        outFile << ")\n";
    }

    outFile << "Classes:\n";
    for (const auto& [name, cls] : classids) 
    {
        outFile << "  Class " << cls.name << "\n";
    }

    outFile << "\n"; 
    outFile.close(); 
}


void SymTable::setValue(const string& name, const Value& value) 
{
    if (ids.find(name) != ids.end()) 
    {
        ids[name].value = value;
    } 
}

SymTable::~SymTable() 
{
    // Curățarea memoriei poate fi realizată aici dacă este necesar
}
