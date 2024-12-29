#ifndef AST_H
#define AST_H
#include <string>
#include <iostream>
using namespace std;

class ASTNode {
    struct Node {
        Node *left;
        Node* rigth;
        string value;
        string type;
    } *root;
    public:
    ASTNode();
    void AddNode(string value,string type, Node* left, Node* right);
    ~ASTNode();
};
#endif

