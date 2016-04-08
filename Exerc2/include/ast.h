#pragma once

#include <iostream>
#include <vector>

namespace AST {

//Binary operations
enum Operation { plus, multiply };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
    public:
        virtual ~Node() {}
        virtual void printTree(){}
        virtual int computeTree(){return 0;}
};

class Integer : public Node {
    public:
        int value;
        Integer(int value) : value(value) {  }
        void printTree();
        int computeTree();
};

class Identifier : public Node {
  public:
    std::string name;
    int value;
    Identifier(std::string name) : name(name), value(0) { }
    Identifier(std::string name, int value) : name(name), value(value) { }
    void printTree();
    int computeTree();

    void setValue(int v){ value = v; }
};

class BinOp : public Node {
    public:
        Operation op;
        Node *left;
        Node *right;
        BinOp(Node *left, Operation op, Node *right) :
            left(left), right(right), op(op) { }
        void printTree();
        int computeTree();
};

class Block : public Node {
    public:
        NodeList lines;
        Block() { }
        void printTree();
        int computeTree();
};

}
