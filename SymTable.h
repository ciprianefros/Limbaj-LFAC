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
        void addParam(const string&type, const string& name) 
        {
            params.emplace_back(type, name);
        }
};

enum Types {
    INT, 
    FLOAT, 
    CHAR, 
    BOOL,
    STRING
};

struct Type {
    short typeName; //0 int 1 float 2 char 3 bool 4 string
    bool isArray;
    vector<short> arraySizes;
};

class Value {
    private:
        int intValue;
        float floatValue;
        char charValue;
        bool boolValue;
        string stringValue;
        short type;

    public:
        Value();
        Value(int x);
        Value(float x);
        Value(string x);
        string toString();
};

class VarInfo {
    public:
        Value value;
        string name;
        Type type;
        VarInfo();
        VarInfo(short type, const string& name);
};

class FuncInfo {
    public:
        string name;
        short returnType;
        ParamList params;

        FuncInfo();
        FuncInfo(short type, const string& name);
};

class ClassInfo {
    public: 
        string name;
        map<string, VarInfo> members;
        map<string, FuncInfo> methods;
        
        ClassInfo() : name("") {}
        ClassInfo(const string& name);
        ~ClassInfo();

};

class SymTable {
    public:
        map<string, VarInfo> ids;
        map<string, FuncInfo> funcids;
        map<string, ClassInfo> classids;
        string ScopeName;
        SymTable* prev;

        SymTable(const string& name, SymTable* parent = nullptr);
        bool existsId(const string& name);
        bool existsFunc(const string& name);
        bool existsClass(const string& name);
        bool addVar(short type, const string& name);
        bool addFunc(short type, const string& name);
        bool addClass(const string& name);
        void printTable(const std::string& filename);
        void setValue(const string& name, const Value& value);
        ~SymTable();
};

#endif // SYMTABLE_H
