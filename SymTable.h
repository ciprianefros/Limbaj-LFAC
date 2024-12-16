#include <iostream>
#include <map>
#include <string>

using namespace std;

class ParamList {
};
class Value {
    private: 
    int type;
    public:
    Value() {};
    Value(int x);
    Value(float x);
    Value(string type);
    string toString();
};

class IdInfo {
    public:
    Value value;
    string idType;
    string type;
    string name;
    ParamList params; //for functions
    IdInfo() {}
    IdInfo(const char* type, const char* name, const char* idType) : type(type),name(name),idType(idType) {}
};



class SymTable {
    map<string, IdInfo> ids;
    map<string, IdInfo> funcids;
    string name;
    SymTable* prev;
    public:
    SymTable(const char* name) :  name(name){}
    bool existsId(const char* s);
    void addVar(const char* type, const char* name );
    void addFunc(const char* type, const char* name);
    void printFuncs();
    void printVars();
    void setValue(const char* name);
    ~SymTable();
};






