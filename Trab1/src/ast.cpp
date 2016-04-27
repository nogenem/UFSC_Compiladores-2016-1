#include "ast.hpp"

using namespace AST;
extern ST::SymbolTable *symtab;

void Block::printTree(){
  for (Node* line: lines) {
      line->printTree();
      std::cout << std::endl;
  }
}

const char* Variable::getType(){
  auto symbol = symtab->getSymbol(id);
  if(symbol == nullptr)
    return "indefinida";

  switch (symbol->type) {
    case ST::integer_t: return "inteira";
    case ST::real_t: return "real";
    case ST::bool_t: return "booleana";
  }
}

void Variable::printTree(){
  if (next != NULL){
      next->printTree();
      std::cout << ", ";
  }else{
    if(declaration)
      std::cout << "Declaracao de variavel " << getType() << ": ";
    else
      std::cout << "variavel " << getType() << " ";
  }
  std::cout << id;
}

void Bool::printTree(){
  std::cout << "valor booleano " << (n?"TRUE":"FALSE");
}

void Integer::printTree(){
  std::cout << "valor inteiro " << n;
}

void Real::printTree(){
  std::cout << "valor real " << n;
}

void Parentheses::printTree(){
  std::cout << "(abre parenteses) ";
  expr->printTree();
  std::cout << " (fecha parenteses)";
}

void UniOp::printTree(){
  switch (op) {
    case u_minus: std::cout << "(menos unario TYPE) "; break;
    case u_not: std::cout << "(nao unario booleano) "; break;
    default: break;
  }
  expr->printTree();
}

void BinOp::printTree(){
  if(op == assign){
    std::cout << "Atribuicao de valor para ";
    left->printTree();
    std::cout << ": ";
    right->printTree();
    return;
  }

  left->printTree();
  switch (op) {
    case plus: std::cout << " (soma TYPE) "; break;
    case b_minus: std::cout << " (subtracao TYPE) "; break;
    case times: std::cout << " (vezes TYPE) "; break;
    case division: std::cout << " (divisao TYPE) "; break;
    default: break;
  }
  right->printTree();
}
