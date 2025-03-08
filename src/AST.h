#ifndef AST_H
#define AST_H
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include "SymTable.h"

using namespace std;

enum B_operation 
{
    ADD, SUB, DIV, MUL, MOD, BLT, BLTE, BGT, BGTE, BEQ, BNEQ, BAND, BOR
};

enum U_operation 
{
    BNOT = 15
};

class ASTNode 
{
    short type;
    short operation;
    ASTNode *left;
    ASTNode *right;
    union 
    {
        int number;
        float real_number;
        bool true_or_false;
        char caracter;
        char* string;
    }valoare;

    public:
        ASTNode();
        ASTNode(int valoare, short type);
        ASTNode(int valoare);
        ASTNode(float valoare);
        ASTNode(bool valoare);
        ASTNode(char valoare);
        ASTNode(char* valoare);    
        ~ASTNode();

        ASTNode (B_operation operation, ASTNode *left, ASTNode *right);
        ASTNode (U_operation operation, ASTNode *nod);
        int GetType();
        int GetIntValue();
        char GetCharValue();
        char* GetStringValue();
        float GetFloatValue();
        bool GetBoolValue();
        void ReduceToOneNode();
};
#endif
