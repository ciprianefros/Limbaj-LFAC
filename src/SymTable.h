#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <stack>
#include <unordered_map>

using namespace std;

extern int yylineno;


enum Types
{
    TYPE_INT, 
    TYPE_FLOAT, 
    TYPE_CHAR, 
    TYPE_BOOL,
    TYPE_STRING,
    CUSTOM_TYPE
};

struct Type 
{
    short typeName; //0 int 1 float 2 char 3 bool 4 string 5 customClass
    string className;
    bool isArray;
    vector<short> arraySizes;
};

class Value 
{
    private:
        int intValue;
        float floatValue;
        char charValue;
        bool boolValue;
        char* stringValue;
        short type;

    public:
        Value();
        Value(int x);
        Value(float x);
        Value(char x);
        Value(bool x);
        Value(char* x);

        int getIntValue();
        float getFloatValue();
        char getCharValue();
        bool getBoolValue();
        char* getStringValue();
        int getType() const;
        void setType(short type);
        string toString();
};

class VarInfo 
{
    public:
        Value value;
        string name;
        Type type;
        vector<VarInfo> fields;

        VarInfo();
        VarInfo(short type, const string& name);
        VarInfo(const VarInfo& v);
        VarInfo(short type, const string& name, Value valoare);
};

struct VarSign
{
    string varName;
    short varIndex[2];
    string varField;
    int varType; // 0 simple, 1 arrayElement, 2 objectField
};

class FuncInfo 
{
    public:
        string name;
        short returnType; //0 int 1 float 2 char 3 bool 4 string 5 customClass
        string className;
        vector<VarInfo> params; 

        FuncInfo();
        FuncInfo(short type, const string& name, vector<VarInfo> params);
        FuncInfo(short type, const string& name, vector<VarInfo> params, const string& className);
        
        
};

class ClassInfo 
{
    public: 
        string name;
        map<string, VarInfo> members;
        map<string, FuncInfo> methods;
        
        ClassInfo() : name("") {}
        ClassInfo(const string& name);
        ~ClassInfo();

};

class SymTable 
{
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
        bool addVar(short type, const string& name, Value valoare);
        bool addVar(const VarInfo& v);
        bool addFunc(short type, const string& name, vector<VarInfo> params);
        bool addFunc(short type, const string& name, vector<VarInfo> params, const string& className);
        bool addClass(const string& name);
        void printArrayValues(VarInfo& var, std::ofstream& outFile, size_t level);
        void printTable(const std::string& filename);
        void setValue(const string& name, const Value& value);
        ~SymTable();
};

#endif // SYMTABLE_H
