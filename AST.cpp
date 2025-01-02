#include "AST.h"
#include <string.h>

extern int yylineno;
bool exists_error = false;

ASTNode::ASTNode()
{
    left =  nullptr;
    right = nullptr;
    operation = -1;
}
ASTNode::ASTNode(int valoare, short type)
{
    left = nullptr;
    right = nullptr;
    operation = -1;
    this->type = type;
    if(type == TYPE_INT)
        this->valoare.number = 0;
    else if(type == TYPE_BOOL)
        this->valoare.true_or_false = false;
    else if(type == TYPE_CHAR)
        this->valoare.caracter = 'n';
    else if(type == TYPE_FLOAT)
        this->valoare.real_number = 0;
    else if(type == TYPE_STRING)
        this->valoare.string = strdup("null");
}

ASTNode::ASTNode(int valoare)
{
    this->valoare.number = valoare;
    type = TYPE_INT;
    left = nullptr;
    right = nullptr;
    operation = -1;
}

ASTNode::ASTNode(float valoare)
{
    this->valoare.real_number = valoare;
    type = TYPE_FLOAT;
    left = nullptr;
    right = nullptr;
    operation = -1;
}

ASTNode::ASTNode(bool valoare)
{
    this->valoare.true_or_false = valoare;
    type = TYPE_BOOL;
    left = nullptr;
    right = nullptr;
    operation = -1;
}

ASTNode::ASTNode(char valoare)
{
    this->valoare.caracter = valoare;
    type = TYPE_CHAR;
    left = nullptr;
    right = nullptr;
    operation = -1;
}

ASTNode::ASTNode(char* valoare)
{
    this->valoare.string = strdup(valoare);
    type = TYPE_STRING;
    left = nullptr;
    right = nullptr;
    operation = -1;
}

ASTNode::~ASTNode()
{
}

ASTNode::ASTNode(B_operation operation, ASTNode *left, ASTNode *right)
{
    this->left = left;
    this->right = right;
    this->operation = operation;
    if(left->type != right->type){
        printf("ERROR LINE:%d\tOperanzii nu au acelasi tip!\n", yylineno);
        exists_error = true;
    }
    if(left->type == TYPE_CHAR) {
        printf("ERROR LINE:%d\tChar nu suporta operatii efectuate asupra sa!\n", yylineno);
        exists_error = true;
    }
    if(left->type == TYPE_STRING) {
        printf("ERROR LINE:%d\tString nu suporta operatii efectuate asupra sa!\n", yylineno);
        exists_error = true;
    }
    if(operation > MOD) {
        type = TYPE_BOOL;
        if(left->type == TYPE_INT && operation > BAND) {
            printf("ERROR LINE:%d\tInt nu suporta urmatoarele operatii efectuate asupra sa: AND si OR!\n", yylineno);
            exists_error = true;
        }
        if(left->type == TYPE_FLOAT && operation > BAND) {
            printf("ERROR LINE:%d\tFloat nu suporta urmatoarele operatii efectuate asupra sa: AND si OR!\n", yylineno);
            exists_error = true;
        }
    }
    else {
        if(left->type == TYPE_BOOL) {
            printf("ERROR LINE:%d\tBool nu suporta urmatoarele operatii efectuate asupra sa: ADD, SUB, DIV, MUL si MOD!\n", yylineno);
            exists_error = true;
        }
        type = left->type;
    }
}

ASTNode::ASTNode(U_operation operation, ASTNode *nod)
{
    this->left = nod;
    this->right = nullptr;
    this->operation = operation;
    if(nod->type != TYPE_BOOL) 
    {
        printf("ERROR LINE:%d\tNu poti folosi operatia NOT decat pe tipul boolean!\n", yylineno);
            exists_error = true;
    }
    type = TYPE_BOOL;
}

int ASTNode::GetType()
{
    return type;
}

int ASTNode::GetIntValue()
{
    ReduceToOneNode();
    return valoare.number;
}

char ASTNode::GetCharValue()
{
    return valoare.caracter;
}

char *ASTNode::GetStringValue()
{
    return valoare.string;
}

float ASTNode::GetFloatValue()
{
    ReduceToOneNode();
    return valoare.real_number;
}

bool ASTNode::GetBoolValue()
{
    ReduceToOneNode();
    return valoare.true_or_false;
}

void ASTNode::ReduceToOneNode()
{
    if (left != nullptr) left->ReduceToOneNode();
    if (right != nullptr) right->ReduceToOneNode();
    if (operation == -1) return;
    switch (operation)
    {
    case ADD:
        if(left->type == TYPE_INT) valoare.number = left->valoare.number + right->valoare.number;
        else valoare.real_number = left->valoare.real_number + right->valoare.real_number;
        break;
    case SUB:
        if(left->type == TYPE_INT) valoare.number = left->valoare.number - right->valoare.number;
        else valoare.real_number = left->valoare.real_number - right->valoare.real_number;
        break;
    case DIV:
        if(left->type == TYPE_INT) {
            if(right->valoare.number == 0) {
                printf("ERROR LINE:%d\tDivision by 0 is imposible\n", yylineno);
                exists_error = true;
            }
            else valoare.number = left->valoare.number / right->valoare.number;
        }
        else {
            if(right->valoare.real_number == 0) {
                printf("ERROR LINE:%d\tDivision by 0 is imposible\n", yylineno);
                valoare.real_number = 0;
                exit(1);
            }
            else valoare.real_number = left->valoare.real_number / right->valoare.real_number;
        }
        break;
    case MUL:
        if(left->type == TYPE_INT) valoare.number = left->valoare.number * right->valoare.number;
        else valoare.real_number = left->valoare.real_number * right->valoare.real_number;
        break;
    case MOD:
        if(right->valoare.number == 0) {
            printf("ERROR LINE:%d\tMOD by 0 is imposible\n", yylineno);
            exists_error = true;
        }
        else valoare.number = left->valoare.number % right->valoare.number;
        break;
    case BLT:
        if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number < right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number < right->valoare.real_number;
        break;
    case BLTE:
        if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number <= right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number <= right->valoare.real_number;
        break;
    case BGT:
        if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number > right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number > right->valoare.real_number;
        break;
    case BGTE:
        if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number >= right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number >= right->valoare.real_number;
        break;
    case BEQ:
        if (left->type == TYPE_CHAR) valoare.true_or_false = left->valoare.caracter == right->valoare.caracter;
        else if(left->type == TYPE_BOOL) valoare.true_or_false = left->valoare.true_or_false == right->valoare.true_or_false;
        else if(left->type == TYPE_STRING) valoare.true_or_false = !strcmp(left->valoare.string, right->valoare.string);            
        else if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number == right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number == right->valoare.real_number;
        break;
    case BNEQ:
        if (left->type == TYPE_CHAR) valoare.true_or_false = left->valoare.caracter != right->valoare.caracter;
        else if(left->type == TYPE_BOOL) valoare.true_or_false = left->valoare.true_or_false != right->valoare.true_or_false;
        else if(left->type == TYPE_STRING) valoare.true_or_false = strcmp(left->valoare.string, right->valoare.string);            
        else if(left->type == TYPE_INT) valoare.true_or_false = left->valoare.number != right->valoare.number;
        else valoare.true_or_false = left->valoare.real_number != right->valoare.real_number;
        break;
    case BAND:
        valoare.true_or_false = left->valoare.true_or_false && right->valoare.true_or_false;
        break;
    case BOR:
        valoare.true_or_false = left->valoare.true_or_false || right->valoare.true_or_false;
        break;
    case BNOT:
        valoare.true_or_false = !left->valoare.true_or_false;
        break;
    default:
        printf("Default\n");
    }
}