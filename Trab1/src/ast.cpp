#include "ast.hpp"

using namespace AST;

void Block::printTree(){
  for (Node* line: lines) {
      line->printTree();
      std::cout << std::endl;
  }
}

void Variable::printTree(){
  if (next != NULL){
      next->printTree();
      std::cout << ", ";
  }else{
    std::cout << "Declaracao de variavel TYPE: ";
  }
  std::cout << id;
}
