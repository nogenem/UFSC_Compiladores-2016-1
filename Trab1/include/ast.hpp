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
};

class Bool : public Node {
  public:
    Bool(bool n) : n(n) {}
    void printTree();

    bool n;
};

class Real : public Node {
  public:
    Real(double n) : n(n) {}
    void printTree();

    double n;
};

class Integer : public Node {
  public:
    Integer(int n) : n(n) {}
    void printTree();

    int n;
};

class Parentheses : public Node {
  public:
    Parentheses(Node* expr) : expr(expr) {}
    void printTree();

    Node* expr;
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next, bool declaration=false):
            id(id), next(next), declaration(declaration) {}
    void printTree();
    const char* getType();

    std::string id;
    Node *next;
    bool declaration;//É a declaração da variavel?
};

class BinOp : public Node {
  public:
    BinOp(Node *left, BinOperation op, Node *right) :
        left(left), op(op), right(right) {}
    void printTree();

    Node *left;
    BinOperation op;
    Node *right;
};

class UniOp : public Node {
  public:
    UniOp(Node *expr, UniOperation op)
      : expr(expr), op(op) {}
    void printTree();

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
