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

// Definiția clasei IdInfo
IdInfo::IdInfo() {}

IdInfo::IdInfo(const string& type, const string& name, const string& idType)
    : type(type), name(name), idType(idType) {}

// Definiția clasei SymTable
SymTable::SymTable(const string& name, SymTable* parent)
    : name(name), prev(parent) {}

bool SymTable::existsId(const string& s) {
    return ids.find(s) != ids.end() || funcids.find(s) != funcids.end();
}

void SymTable::addVar(const string& type, const string& name) {
    ids[name] = IdInfo(type, name, "var");
}

void SymTable::addFunc(const string& type, const string& name) {
    funcids[name] = IdInfo(type, name, "func");
}

void SymTable::printVars() {
    for (const auto& var : ids) {
        cout << "Variable: " << var.second.name << " of type " << var.second.type << endl;
    }
}

void SymTable::printFuncs() {
    for (const auto& func : funcids) {
        cout << "Function: " << func.second.name << " of type " << func.second.type << endl;
    }
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
