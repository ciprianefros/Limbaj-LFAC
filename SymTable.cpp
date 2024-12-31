#include "SymTable.h"
#include <string.h>

// Definiția clasei Value
Value::Value() : type(0) {}

Value::Value(int x) : intValue(x), type(1) {}

Value::Value(float x) : floatValue(x), type(2) {}

Value::Value(string x) : stringValue(x), type(3) {}

string Value::toString() {
    if (type == 1) return to_string(intValue);
    if (type == 2) return to_string(floatValue);
    return stringValue;
}

// Definiția clasei VarInfo
VarInfo::VarInfo() {}

VarInfo::VarInfo(short type, const string& name)
{
    this->type.typeName = type;
    this->name = name;
}

// Definiția clasei FuncInfo
FuncInfo::FuncInfo() {}

FuncInfo::FuncInfo(short type, const string& name)
{
    this->returnType = type;
    this->name = name;
}

//Definitia clasei ClassInfo
ClassInfo::ClassInfo(const string& name) : name(name) {}

ClassInfo::~ClassInfo() {
    // Curățare sau alte operațiuni dacă este necesar
}

// Definiția clasei SymTable
SymTable::SymTable(const string& name, SymTable* parent)
    : ScopeName(name), prev(parent) {}



bool SymTable::existsId(const string& name) {
    if(ids.find(name) != ids.end()) {
        return true;
    }
    return prev ? prev->existsId(name) : false;
}
bool SymTable::existsFunc(const string& name) {
    if(funcids.find(name) != funcids.end()) {
        return true;
    }
    return prev ? prev->existsId(name) : false;
}
bool SymTable::existsClass(const string& name) {
    if(classids.find(name) != classids.end()) {
        return true;
    }
    return prev ? prev->existsId(name) : false;
}

bool SymTable::addVar(short type, const string& name) {
    if(ids.find(name) != ids.end()) {
        return false;
    }
    ids[name] = VarInfo(type, name);
    return true;
}

bool SymTable::addFunc(short type, const string& name) {
    if(funcids.find(name) != funcids.end()) {
        return false;
    }
    funcids[name] = FuncInfo(type, name);
    return true;
}
bool SymTable::addClass(const string& name) {
    if(classids.find(name) != classids.end()) {
        return false;
    }
    classids[name] = ClassInfo(name);
    return true;
}

void SymTable::printTable(const string& filename) {
    std::ofstream outFile(filename, ios::app); // Deschide fișierul în modul append

    if (!outFile) {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    outFile << "Scope: " << ScopeName << "\n";
    outFile << "Variabile:\n";
    for (const auto& [name, var] : ids) {
        outFile << "  " << var.type.typeName << " " << var.name << "\n";
    }

    outFile << "Functions:\n";
    for (const auto& [name, func] : funcids) {
        outFile << "  " << func.returnType << " " << func.name << " (";

        for (const auto& [paramType, paramName] : func.params.params) {
            outFile << paramType << " " << paramName;
            if (&paramType != &func.params.params.back().first) // Evită virgula pentru ultimul parametru
                outFile << ", ";
        }
        outFile << ")\n";
    }

    outFile << "Classes:\n";
    for (const auto& [name, cls] : classids) {
        outFile << "  Class " << cls.name << "\n";
    }

    outFile << "\n"; // Linie separatoare pentru claritate
    outFile.close(); // Închide fișierul
}


void SymTable::setValue(const string& name, const Value& value) {
    if (ids.find(name) != ids.end()) {
        ids[name].value = value;
    } 
}

SymTable::~SymTable() {
    // Curățarea memoriei poate fi realizată aici dacă este necesar
}
