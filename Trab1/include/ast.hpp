#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "st.hpp"
#include "util.hpp"

namespace AST {

enum Use { attr, declr, def, read, param };
enum NodeType { node_nt, block_nt, value_nt, variable_nt, array_nt,
  function_nt, return_nt, binop_nt, uniop_nt };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    Node(){}
    Node(Types::Type type): type(type){}
    virtual ~Node() {}
    virtual void printTree(){}
    virtual NodeType getNodeType(){return node_nt;}

    Types::Type type;
};

class Block : public Node{
  public:
    Block(ST::SymbolTable *symtab):
      symtab(symtab){}
    void printTree();
    NodeType getNodeType(){return block_nt;}

    NodeList lines;
    ST::SymbolTable *symtab;
};

class Value : public Node {
  public:
    Value(std::string n, Types::Type type):
      n(n), Node(type){}
    void printTree();
    NodeType getNodeType(){return value_nt;}

    std::string n;
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next, Use use,
      Types::Type type=Types::unknown_t):
      id(id), next(next), use(use), Node(type){}
    void printTree();
    virtual NodeType getNodeType(){return variable_nt;}

    void setType(Types::Type t){type=t;}
    virtual Kinds::Kind getKind(){return Kinds::variable_t;}
    virtual bool equals(Variable *var, bool checkNext=false);

    std::string id;
    Node *next;
    Use use;
};

class Array : public Variable {
  public:
    Array(std::string id, Node *next, Use use):
      Array(id,next,nullptr,use,1,Types::unknown_t){}

    Array(std::string id, Node *index, Use use, Types::Type type):
      Array(id,nullptr,index,use,1,type){}

    Array(std::string id, Node *next, Use use,
      int aSize, Types::Type type):
      Array(id,next,nullptr,use,aSize,type){}

    Array(std::string id, Node *next, Node *i,
      Use use, int aSize, Types::Type type);
    void printTree();
    NodeType getNodeType(){return array_nt;}

    void setSize(int n);
    Kinds::Kind getKind(){return Kinds::array_t;}
    bool equals(Variable *var, bool checkNext=false);

    Node *index;
    int size=0;
};

class Function : public Variable {
  public:
    Function(std::string id, Node *params, Node *block, Use use,
      Types::Type type=Types::unknown_t);
    void printTree();
    NodeType getNodeType(){return function_nt;}

    Kinds::Kind getKind(){return Kinds::function_t;}
    bool equals(Variable *var, bool checkNext=false);

    Node *params;
    Node *block;
};

class Return : public Node {
  public:
    Return(Node *expr):expr(expr), Node(expr->type){}

    void printTree();
    NodeType getNodeType(){return return_nt;}

    Node *expr;
    Types::Type funcType=Types::unknown_t;
};

class BinOp : public Node {
  public:
    BinOp(Node *left, Ops::Operation op, Node *right);

    void printTree();
    NodeType getNodeType(){return binop_nt;}

    Node *left, *right;
    Ops::Operation op;
};

class UniOp : public Node {
  public:
    UniOp(Ops::Operation op, Node *right);

    void printTree();
    NodeType getNodeType(){return uniop_nt;}

    Node *right;
    Ops::Operation op;
};

}
