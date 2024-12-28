#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class ParamList {
    // Poți adăuga aici parametrii funcției dacă ai nevoie
};

class Value {
    private:
        int intValue;
        float floatValue;
        string stringValue;
        int type;

    public:
        Value();
        Value(int x);
        Value(float x);
        Value(string x);
        string toString();
};

class IdInfo {
    public:
        Value value;
        string idType;
        string type;
        string name;
        ParamList params;

        IdInfo();
        IdInfo(const string& type, const string& name, const string& idType);
};

class SymTable {
    public:
        map<string, IdInfo> ids;
        map<string, IdInfo> funcids;
        string name;
        SymTable* prev;

        SymTable(const string& name, SymTable* parent = nullptr);
        bool existsId(const string& s);
        void addVar(const string& type, const string& name);
        void addFunc(const string& type, const string& name);
        void printVars();
        void printFuncs();
        void setValue(const string& name, const Value& value);
        ~SymTable();
};

#endif // SYMTABLE_H
