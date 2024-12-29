#include "SymTable.h"

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
//Definitia clasei ClassInfo
ClassInfo::ClassInfo(const string& name) : name(name) {}

ClassInfo::~ClassInfo() {
    // Curățare sau alte operațiuni dacă este necesar
}
// Definiția clasei IdInfo
IdInfo::IdInfo() {}

IdInfo::IdInfo(const string& type, const string& name, const string& idType)
    : type(type), name(name), idType(idType) {}

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

bool SymTable::addVar(const string& type, const string& name) {
    if(ids.find(name) != ids.end()) {
        return false;
    }
    ids[name] = IdInfo(type, name, "var");
    return true;
}

bool SymTable::addFunc(const string& type, const string& name) {
    if(funcids.find(name) != funcids.end()) {
        return false;
    }
    funcids[name] = IdInfo(type, name, "func");
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
        outFile << "  " << var.type << " " << var.name << "\n";
    }

    outFile << "Functions:\n";
    for (const auto& [name, func] : funcids) {
        outFile << "  " << func.type << " " << func.name << " (";

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
    } else if (funcids.find(name) != funcids.end()) {
        funcids[name].value = value;  // Poți adăuga gestionarea valorii funcțiilor, dacă e necesar
    }
}

SymTable::~SymTable() {
    // Curățarea memoriei poate fi realizată aici dacă este necesar
}
