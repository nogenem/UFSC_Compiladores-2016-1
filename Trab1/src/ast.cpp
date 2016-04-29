#include "ast.hpp"

using namespace AST;
extern ST::SymbolTable *symtab;

BinOp::BinOp(Node *left, BinOperation op, Node *right) :
    left(left), op(op), right(right) {

    auto l = left->getType();
    auto r = right->getType();

    switch (op) {
      case plus:
      case b_minus:
      case times:
      case division:
      case grt:
      case lst:
      case grteq:
      case lsteq:{
        if(l==ST::bool_t || l==ST::notype_t || r==ST::bool_t || r==ST::notype_t){
          yyerror("Operacao binaria '%s' requer dois valores numericos!\n", getOpTxt());
        }
        break;
      }
      case b_and:
      case b_or:{
        if(l!=ST::bool_t || r!=ST::bool_t){
          yyerror("Operacao binaria '%s' requer dois valores booleanos!\n", getOpTxt());
        }
        break;
      }
      case eq:
      case neq:{
        if( (l==ST::bool_t && r!=ST::bool_t) || (r==ST::bool_t && l!=ST::bool_t) ){
          yyerror("Operacao binaria '%s' usando tipos diferentes!\n", getOpTxt());
        }
        break;
      }
      case assign:{
        if(l != r && !(l==ST::real_t && r==ST::integer_t)){
          yyerror("Operacao binaria '%s' usando tipos diferentes!\n", getOpTxt());
        }
        break;
      }
      default: break;
    }
}

UniOp::UniOp(Node *expr, UniOperation op)
  : expr(expr), op(op) {

    auto type = expr->getType();
    switch (op) {
      case u_not:{
        if(type!=ST::bool_t)
          yyerror("Operacao unaria nao requer um valor booleano!\n");
        break;
      }
      case u_minus:{
        if(type==ST::bool_t || type==ST::notype_t)
          yyerror("Operacao unaria menos requer um valor numerico!\n");
        break;
      }
      default: break;
    }
}

ST::Type Parentheses::getType(){
  return expr->getType();
}

ST::Type Variable::getType(){
  auto symbol = symtab->getSymbol(id);
  if(symbol != nullptr){
    return symbol->type;
  }
  return ST::notype_t;
}

const char* Variable::getTypeTxt(){
  switch(getType()){
    case ST::integer_t: return "inteira";
    case ST::real_t: return "real";
    case ST::bool_t: return "booleana";
    case ST::notype_t: return "indefinida";
  }
}

ST::Type BinOp::getType(){
  if(op == assign)
    return ST::notype_t;
  else if(op==b_and || op==b_or || op==grt || op==lst || op==grteq
      || op==lsteq || op==eq || op==neq)
    return ST::bool_t;

  auto l = left->getType();
  auto r = right->getType();

  ST::Type t = ST::integer_t;
  if(l==ST::real_t || r==ST::real_t){
    t = ST::real_t;
  }
  return t;
}

const char* BinOp::getTypeTxt(){
  if(op==grt || op==lst || op==grteq || op==lsteq ||
      op==eq || op==neq){
    auto l = left->getType();
    auto r = right->getType();

    if(l==ST::real_t || r==ST::real_t)
      return "real";
    else if(l==ST::bool_t && r==ST::bool_t)
      return "booleano";
    else
      return "inteiro";
  }
  switch(getType()){
    case ST::real_t: return "real";
    case ST::bool_t: return "booleano";
    case ST::integer_t:{
      switch(op){
        case plus:
        case division:
          return "inteira";
        default: return "inteiro";
      }
    }
    case ST::notype_t: return "indefinida";
  }
}

const char* BinOp::getOpTxt(){
  switch (op) {
    case plus: return "soma";
    case b_minus: return "subtracao";
    case times: return "vezes";
    case division: return "divisao";
    case assign: return "atribuicao";
    case eq: return "igual";
    case neq: return "diferente";
    case grt: return "maior";
    case lst: return "menor";
    case grteq: return "maior ou igual";
    case lsteq: return "menor ou igual";
    case b_and: return "e";
    case b_or: return "ou";
  }
}

ST::Type UniOp::getType(){
  return expr->getType();
}

const char* UniOp::getTypeTxt(){
  if(op==u_not)
    return "booleano";

  switch (getType()) {
    case ST::integer_t: return "inteiro";
    case ST::real_t: return "real";
    case ST::notype_t: return "indefinido";
    default: return "indefinido";
  }
}

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
    if(declaration)
      std::cout << "Declaracao de variavel " << getTypeTxt() << ": ";
    else
      std::cout << "variavel " << getTypeTxt() << " ";
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
    case u_minus: std::cout << "(menos unario "<< getTypeTxt() <<") "; break;
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

  auto l = left->getType();
  auto r = right->getType();

  left->printTree();
  if(r==ST::real_t && l==ST::integer_t)
    std::cout << " para real";

  switch (op) {
    case plus: std::cout << " (soma "; break;
    case b_minus: std::cout << " (subtracao "; break;
    case times: std::cout << " (vezes "; break;
    case division: std::cout << " (divisao "; break;
    case eq: std::cout << " (igual "; break;
    case neq: std::cout << " (diferente "; break;
    case grt: std::cout << " (maior "; break;
    case lst: std::cout << " (menor "; break;
    case grteq: std::cout << " (maior ou igual "; break;
    case lsteq: std::cout << " (menor ou igual "; break;
    case b_and: std::cout << " (e "; break;
    case b_or: std::cout << " (ou "; break;
    default: break;
  }
  std::cout << getTypeTxt() << ") ";
  right->printTree();
  if(l==ST::real_t && r==ST::integer_t)
    std::cout << " para real";
}
