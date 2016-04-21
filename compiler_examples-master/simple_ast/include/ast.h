/*Abstract Syntax Tree definition*/
#pragma once

#include <iostream>
#include <vector>
#include "Variant_t.h"
#include "st.h"

extern void yyerror(const char *s, ...);

namespace AST {

//Binary operations
enum Operation { plus, times, assign };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
    public:
        virtual ~Node() {}
        virtual void printTree(){}
        virtual VAR::Variant_t computeTree(ST::SymbolTable *scope){return VAR::Variant_t();}
};

class Integer : public Node {
    public:
        VAR::Variant_t value;
        Integer(int value) : value(value) {  }
        void printTree();
        VAR::Variant_t computeTree(ST::SymbolTable *scope);
};

class Double : public Node {
    public:
        VAR::Variant_t value;
        Double(double value) : value(value) {  }
        void printTree();
        VAR::Variant_t computeTree(ST::SymbolTable *scope);
};

class BinOp : public Node {
    public:
        Operation op;
        Node *left;
        Node *right;
        BinOp(Node *left, Operation op, Node *right) :
            left(left), right(right), op(op) { }
        void printTree();
        VAR::Variant_t computeTree(ST::SymbolTable *scope);
};

class Block : public Node {
    public:
        NodeList lines;
        ST::SymbolTable *scope;

        Block(ST::SymbolTable *scope) : scope(scope) {}
        void printTree();
        VAR::Variant_t computeTree(ST::SymbolTable *scope);
};

class Variable : public Node {
     public:
         std::string id;
         Node *next;
         
         Variable(std::string id, Node *next) :
              id(id), next(next) { }
         void printTree();
         VAR::Variant_t computeTree(ST::SymbolTable *scope);
};

}
