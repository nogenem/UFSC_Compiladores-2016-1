#pragma once

#include <vector>
#include <iostream>
#include "st.hpp"

extern void yyerror(const char* s, ...);

namespace AST {

//Binary operations
enum BinOperation { plus, b_minus, times, division, assign, eq, neq, grt, grteq, lst, lsteq, b_and, b_or };
enum UniOperation { u_not, u_minus };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    virtual ~Node(){}
    virtual void printTree(){}
    virtual ST::Type getType(){return ST::notype_t;}
    virtual const char* getTypeTxt(){return "indefinido";}
};

class Bool : public Node {
  public:
    Bool(bool n) : n(n) {}
    void printTree();
    ST::Type getType(){return ST::bool_t;}
    const char* getTypeTxt(){return "booleano";}

    bool n;
};

class Real : public Node {
  public:
    Real(double n) : n(n) {}
    void printTree();
    ST::Type getType(){return ST::real_t;}
    const char* getTypeTxt(){return "real";}

    double n;
};

class Integer : public Node {
  public:
    Integer(int n) : n(n) {}
    void printTree();
    ST::Type getType(){return ST::integer_t;}
    const char* getTypeTxt(){return "inteiro";}

    int n;
};

class Parentheses : public Node {
  public:
    Parentheses(Node* expr) : expr(expr) {}
    void printTree();
    ST::Type getType();

    Node* expr;
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next, bool declaration=false):
            id(id), next(next), declaration(declaration) {}
    virtual void printTree();
    virtual ST::Type getType();
    virtual const char* getTypeTxt();

    std::string id;
    Node *next;
    bool declaration;//É a declaração da variavel?
};

class Array : public Variable {
  public:
    Array(std::string id, Node *next, bool declaration=false) :
        Variable(id, next, declaration){}

    void printTree();
    const char* getTypeTxt();

    int getSize();
};

class BinOp : public Node {
  public:
    BinOp(Node *left, BinOperation op, Node *right);
    void printTree();
    ST::Type getType();
    const char* getTypeTxt();
    const char* getOpTxt();

    Node *left;
    BinOperation op;
    Node *right;
};

class UniOp : public Node {
  public:
    UniOp(Node *expr, UniOperation op);
    void printTree();
    ST::Type getType();
    const char* getTypeTxt();

    Node *expr;
    UniOperation op;
};

class Block : public Node{
  public:
    Block(){}
    void printTree();

    NodeList lines;
};

}
