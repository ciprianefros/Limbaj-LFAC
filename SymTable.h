#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <fstream>

using namespace std;

class ParamList {
    public: 
        vector<pair<string, string>> params; //vector de (tip, nume)
        void addParam(const string&type, const string& name) {
            params.emplace_back(type, name);
        }
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

class ClassInfo {
    public: 
        string name;
        map<string, IdInfo> members;
        map<string, IdInfo> methods;
        
        ClassInfo() : name("") {}
        ClassInfo(const string& name);
        ~ClassInfo();

};

class SymTable {
    public:
        map<string, IdInfo> ids;
        map<string, IdInfo> funcids;
        map<string, ClassInfo> classids;
        string ScopeName;
        SymTable* prev;

        SymTable(const string& name, SymTable* parent = nullptr);
        bool existsId(const string& name);
        bool existsFunc(const string& name);
        bool existsClass(const string& name);
        bool addVar(const string& type, const string& name);
        bool addFunc(const string& type, const string& name);
        bool addClass(const string& name);
        void printTable(const std::string& filename);
        void setValue(const string& name, const Value& value);
        ~SymTable();
};

#endif // SYMTABLE_H
