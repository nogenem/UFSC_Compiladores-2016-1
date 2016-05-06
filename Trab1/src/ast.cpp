#include "ast.hpp"
#include <string>
#include <iomanip>

using namespace AST;
extern ST::SymbolTable *symtab;

BinOp::BinOp(Node *left, BinOperation op, Node *right) :
    left(left), op(op), right(right) {

    auto l = left->getType();
    auto r = right->getType();

    auto txt1 = std::string(left->getTypeTxt());
    auto txt2 = std::string(right->getTypeTxt());

    // gambiarra pra deixar tudo masculino!!!!
    if(txt1 != std::string("real"))
      txt1 = txt1.substr(0, txt1.size()-1) + "o";
    if(txt2 != std::string("real"))
      txt2 = txt2.substr(0, txt2.size()-1) + "o";

    switch (op) {
      case plus:
      case b_minus:
      case times:
      case division:
      case grt:
      case lst:
      case grteq:
      case lsteq:{
        if(l==ST::bool_t || l==ST::notype_t){
          yyerror("semantico: operacao %s espera inteiro ou real mas recebeu %s.\n",
            getOpTxt(), txt1.c_str());
        }
        if(r==ST::bool_t || r==ST::notype_t){
          yyerror("semantico: operacao %s espera inteiro ou real mas recebeu %s.\n",
            getOpTxt(), txt2.c_str());
        }
        break;
      }
      case b_and:
      case b_or:{
        if(l!=ST::bool_t){
          yyerror("semantico: operacao %s espera booleano mas recebeu %s.\n",
            getOpTxt(), txt1.c_str());
        }
        if(r!=ST::bool_t){
          yyerror("semantico: operacao %s espera booleano mas recebeu %s.\n",
            getOpTxt(), txt2.c_str());
        }
        break;
      }
      case eq:
      case neq:{
        if( l==ST::bool_t || l==ST::notype_t ){
          yyerror("semantico: operacao %s espera inteiro ou real mas recebeu %s.\n",
            getOpTxt(), txt1.c_str());
        }
        if( r==ST::bool_t || r==ST::notype_t ){
          yyerror("semantico: operacao %s espera inteiro ou real mas recebeu %s.\n",
            getOpTxt(), txt2.c_str());
        }
        break;
      }
      case assign:{
        if(l != r && !(l==ST::real_t && r==ST::integer_t)){
          yyerror("semantico: operacao %s espera %s mas recebeu %s.\n",
            getOpTxt(), txt1.c_str(), txt2.c_str());
        }
        break;
      }
      default: break;
    }
}

UniOp::UniOp(Node *expr, UniOperation op)
  : expr(expr), op(op) {

    auto type = expr->getType();
    auto txt = std::string(expr->getTypeTxt());
    // gambiarra pra deixar tudo masculino!!!!
    if(txt != std::string("real"))
      txt = txt.substr(0, txt.size()-1) + "o";

    switch (op) {
      case u_not:{
        if(type!=ST::bool_t){
          yyerror("semantico: operacao %s espera booleano mas recebeu %s.\n",
            getOpTxt(), txt.c_str());
        }
        break;
      }
      case u_minus:{
        if(type==ST::bool_t || type==ST::notype_t){
          yyerror("semantico: operacao %s espera inteiro ou real mas recebeu %s.\n",
              getOpTxt(), txt.c_str());
        }
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
    case ST::notype_t: return "desconhecida";
  }
}

const char* Array::getTypeTxt(){
  switch(getType()){
    case ST::integer_t: return "inteiro";
    case ST::real_t: return "real";
    case ST::bool_t: return "booleano";
    case ST::notype_t: return "desconhecido";
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
  if( (l!=ST::notype_t && r!=ST::notype_t) &&
    (l==ST::real_t || r==ST::real_t) ){
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
    else
      return (op==eq || op==neq) ? "booleano" : "inteiro";
  }
  switch(getType()){
    case ST::real_t: return "real";
    case ST::bool_t: return "booleano";
    case ST::integer_t:{
      switch(op){
        case plus:
        case b_minus:
        case division:
        case times:
          return "inteira";
        default: return "inteiro";
      }
    }
    case ST::notype_t: return "desconhecida";
  }
}

const char* BinOp::getOpTxt(){
  switch (op) {
    case plus: return "soma";
    case b_minus: return "subtracao";
    case times: return "multiplicao";
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

const char* UniOp::getOpTxt(){
  switch (op) {
    case u_not: return "nao";
    case u_minus: return "menos";
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
    case ST::bool_t: return "booleano";
    case ST::notype_t: return "desconhecido";
    default: return "desconhecido";
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
    switch (use) {
      case declr:{
        std::cout << "Declaracao de variavel " << getTypeTxt() << ": ";
        break;
      }case attr:{
        std::cout << "Atribuicao de valor para variavel " <<
          getTypeTxt() << " ";
        break;
      }case read:{
        std::cout << "variavel " << getTypeTxt() << " ";
        break;
      }
    }
  }
  std::cout << id;
}

int Array::getSize(){
  auto symbol = symtab->getSymbol(id);
  if (symbol != nullptr)
    return symbol->aSize;
  else
    return 0;
}

void Array::printTree(){
  if (next != NULL){
      next->printTree();
      std::cout << ", ";
  }else{
    switch(use){
      case declr:{
        std::cout << "Declaracao de arranjo " << getTypeTxt()
          << " de tamanho " << getSize() << ": ";
        break;
      }case attr:{
        std::cout << "Atribuicao de valor para arranjo " <<
            getTypeTxt() << " " << id << ":\n+indice: ";
        index->printTree();
        std::cout << "\n+valor";
        break;
      }case read:{
        std::cout << "arranjo " << getTypeTxt() << " ";
        break;
      }
    }
  }
  if(use != attr)
    std::cout << id;
}

void Bool::printTree(){
  std::cout << "valor booleano " << (n?"TRUE":"FALSE");
}

void Integer::printTree(){
  std::cout << "valor inteiro " << n;
}

void Real::printTree(){
  std::cout << "valor real " <<
    std::setiosflags(std::ios::fixed) << std::setprecision(2) << n;
}

void Parentheses::printTree(){
  std::cout << "((abre parenteses) ";
  expr->printTree();
  std::cout << " (fecha parenteses))";
}

void UniOp::printTree(){
  std::cout << "(";
  switch (op) {
    case u_minus: std::cout << "(menos unario "<< getTypeTxt() <<") "; break;
    case u_not: std::cout << "(nao unario booleano) "; break;
    default: break;
  }
  expr->printTree();
  std::cout << ")";
}

void BinOp::printTree(){
  if(op == assign){
    left->printTree();
    std::cout << ": ";
    right->printTree();
    return;
  }

  auto l = left->getType();
  auto r = right->getType();

  std::cout << "(";
  left->printTree();
  if(r==ST::real_t && l==ST::integer_t)
    std::cout << " para real";

  switch (op) {
    case plus: std::cout << " (soma "; break;
    case b_minus: std::cout << " (subtracao "; break;
    case times: std::cout << " (multiplicacao "; break;
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
  std::cout << ")";
}
