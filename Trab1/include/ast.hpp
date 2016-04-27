#pragma once

#include <vector>
#include <iostream>

extern void yyerror(const char* s, ...);

namespace AST {

//Binary operations
enum BinOperation { plus, b_minus, times, assign, eq, neq, grt, grteq, lst, lsteq, b_and, b_or };
enum UniOperation { u_not, u_minus };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    virtual ~Node(){}
    virtual void printTree(){}
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next):
            id(id), next(next) {}
    void printTree();

    std::string id;
    Node *next;
};

class Block : public Node{
  public:
    Block(){}
    void printTree();

    NodeList lines;
};

}
