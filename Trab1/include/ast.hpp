#pragma once

#include <vector>
#include <iostream>

extern void yyerror(const char* s, ...);

namespace AST {

//Binary operations
enum Operation { plus, times, assign };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    Node(){}
};

}
