#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "st.hpp"
#include "util.hpp"

extern void yyerror(const char* s, ...);

namespace AST {

enum Use { attr, declr, read, param };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    Node(){}
    Node(Types::Type type): type(type){}
    virtual ~Node() {}
    virtual void printTree(){}

    Types::Type type;
};

class Block : public Node{
  public:
    Block(ST::SymbolTable *symtab):
      symtab(symtab){}
    void printTree();

    NodeList lines;
    ST::SymbolTable *symtab;
};

class Value : public Node {
  public:
    Value(std::string n, Types::Type type):
      n(n), Node(type){}
    void printTree();

    std::string n;
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next, Use use,
      Types::Type type=Types::unknown_t):
      id(id), next(next), use(use), Node(type){}
    void printTree();
    void setType(Types::Type t){type=t;}

    std::string id;
    Node *next;
    Use use;
};

class Array : public Variable {
  public:
    Array(std::string id, Node *next, Use use,
      Types::Type type=Types::unknown_t):
      Variable(id,next,use,type){}

    Array(std::string id, Node *next, Use use, int aSize,
      Types::Type type=Types::unknown_t):
      Variable(id,next,use,type){ size = aSize; }

    Array(std::string id, Node *next, Node *i, Use use,
      Types::Type type=Types::unknown_t):
      Variable(id,next,use,type){index = i;}

    void printTree();
    void setSize(int n){size = n;}

    Node *index;
    int size=0;
};

class Function : public Variable {
  public:
    Function(std::string id, Node *params, Node *block, Use use,
      Types::Type type=Types::unknown_t):
      params(params), block(block), Variable(id,nullptr,use,type){}

    void printTree();

    Node *params;
    Node *block;
};

class BinOp : public Node {
  public:
    BinOp(Node *left, Ops::Operation op, Node *right);

    void printTree();

    Node *left, *right;
    Ops::Operation op;
};

class UniOp : public Node {
  public:
    UniOp(Ops::Operation op, Node *right);

    void printTree();

    Node *right;
    Ops::Operation op;
};

}
