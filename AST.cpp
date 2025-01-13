#include "AST.h"
#include <string.h>

extern int yylineno;
extern int errorCount;
void yyerror(const char * s);

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
        this->valoare.number = valoare;
    else if(type == TYPE_BOOL)
        this->valoare.true_or_false = valoare != 0;
    else if(type == TYPE_CHAR)
        this->valoare.caracter = valoare;
    else if(type == TYPE_FLOAT)
        this->valoare.real_number = valoare;
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
    delete left;
    delete right;
}

ASTNode::ASTNode(B_operation operation, ASTNode *left, ASTNode *right)
{
    this->left = left;
    this->right = right;
    this->operation = operation;
    if(left->type != right->type ){
        yyerror("Operanzii nu au acelasi tip!");
        errorCount++;
    }
    if(left->type == TYPE_CHAR) {
        yyerror("Char nu suporta operatii efectuate asupra sa! ");
        errorCount++;
    }
    if(left->type == TYPE_STRING) {
        yyerror("String nu suporta operatii efectuate asupra sa! ");
        errorCount++;
    }
    if(operation > MOD) {
        type = TYPE_BOOL;
        if(left->type == TYPE_INT && operation > BAND) {
            yyerror("Int nu suporta urmatoarele operatii efectuate asupra sa: AND si OR! ");
            errorCount++;
        }
        if(left->type == TYPE_FLOAT && operation > BAND) {
            yyerror("Float nu suporta urmatoarele operatii efectuate asupra sa: AND si OR! ");
            errorCount++;
        }
    }
    else {
        if(left->type == TYPE_BOOL) {
            yyerror("Bool nu suporta urmatoarele operatii efectuate asupra sa: ADD, SUB, DIV, MUL si MOD! ");
            errorCount++;
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
        yyerror("Poti folosi operatia NOT decat pe tipul boolean! ");
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
                yyerror("Împărțirea la 0 este imposibilă. ");
                errorCount++;
            }
            else valoare.number = left->valoare.number / right->valoare.number;
        }
        else {
            if(right->valoare.real_number == 0) {
                yyerror("Împărțirea la 0 este imposibilă. ");
                valoare.real_number = 0;
                errorCount++;
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
            yyerror("Împărțirea MOD 0 nu este posibilă! ");
            errorCount++;
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
        yyerror("Default ERROR");
    }
}