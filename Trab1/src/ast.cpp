#include "ast.hpp"

using namespace AST;

Array::Array(std::string id, Node *next, Node *i,
  Use use, int aSize, Types::Type type):
  Variable(id,next,use,type){

  if(aSize < 1){
    aSize = 1;
    Errors::print(Errors::array_index_lst_1, id.c_str());
  }
  size = aSize;
  index = i;
}

void Array::setSize(int n){
  if(n < 1){
    n = 1;
    Errors::print(Errors::array_index_lst_1, id.c_str());
  }
  size = n;
}

BinOp::BinOp(Node *left, Ops::Operation op, Node *right):
  left(left), op(op), right(right){

  auto l = left->type;
  auto ltxt = Types::mascType[l];
  auto r = right->type;
  auto rtxt = Types::mascType[r];
  auto optxt = Ops::opName[op];

  switch (op) {
    case Ops::assign:
      type = l;
      if(l != r && !(l==Types::real_t && r==Types::integer_t))
        Errors::print(Errors::op_wrong_type1, optxt, ltxt, rtxt);
      break;

    case Ops::plus:
    case Ops::b_minus:
    case Ops::times:
    case Ops::division:{
      type = Types::integer_t;
      if( (l==Types::real_t && r==Types::real_t) ||
          (l==Types::real_t && r==Types::integer_t) ||
          (l==Types::integer_t && r==Types::real_t)){
        type = Types::real_t;
      }
      if(l!=Types::integer_t && l!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, ltxt);
      if(r!=Types::integer_t && r!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;

    }case Ops::eq:
    case Ops::neq:
    case Ops::grt:
    case Ops::grteq:
    case Ops::lst:
    case Ops::lsteq:
      type = Types::bool_t;
      if(l!=Types::integer_t && l!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, ltxt);
      if(r!=Types::integer_t && r!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;

    case Ops::b_and:
    case Ops::b_or:
      type = Types::bool_t;
      if(l!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], ltxt);
      if(r!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], rtxt);
      break;

    default: type = Types::unknown_t;
  }
}

UniOp::UniOp(Ops::Operation op, Node *right):
  op(op), right(right) {

  auto r = right->type;
  auto rtxt = Types::mascType[r];
  auto optxt = Ops::opName[op];

  switch (op) {
    case Ops::u_not:
      type = Types::bool_t;
      if(r!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], rtxt);
      break;

    case Ops::u_minus:
      type = Types::integer_t;
      if(r==Types::real_t || r==Types::integer_t)
        type = r;
      else
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;

    case Ops::u_paren:
      type = r;
      break;

    default: type = Types::unknown_t;
  }
}

void Value::printTree(){
  std::cout << "valor " << Types::mascType[type] << " " << n;
}

void Variable::printTree(){
  if(use == param){
    std::cout << "parametro " << Types::mascType[type] << ": " << id << "\n";
    if(next != NULL) next->printTree();
  }else{
    if(next != NULL){
      next->printTree();
      std::cout << ", ";
    }else{
      switch (use) {
        case declr:
          std::cout << "Declaracao de variavel " << Types::femType[type] << ": ";
          break;
        case attr:
          std::cout << "Atribuicao de valor para variavel " <<
            Types::femType[type]  << " ";
          break;
        case read:
          std::cout << "variavel " << Types::femType[type] << " ";
          break;
        default: break;
      }
    }
    std::cout << id;
  }
}

void Array::printTree(){
  if(use == param){
    std::cout << "parametro " << Types::mascType[type] << " de tamanho " <<
      size << ": " << id << "\n";
    if(next != NULL) next->printTree();
  }else{
    if(next != NULL){
      next->printTree();
      std::cout << ", ";
    }else{
      switch (use) {
        case declr:
          std::cout << "Declaracao de arranjo " << Types::mascType[type]
            << " de tamanho " << size << ": ";
          break;
        case attr:
          std::cout << "Atribuicao de valor para arranjo " <<
            Types::mascType[type]  << " " << id << ":\n+indice: ";
          index->printTree();
          std::cout << "\n+valor";
          break;
        case read:
          std::cout << "arranjo " << Types::mascType[type] << " " << id <<
            " {+indice: ";
          index->printTree();
          std::cout << "}";
          break;
        default: break;
      }
    }
    if(use == declr)
      std::cout << id;
  }
}

void Function::printTree(){
  switch (use) {
    case declr:
      std::cout << "Declaracao de funcao " << Types::femType[type] <<
        ": " << id << "\n+parametros:\n";
      params->printTree();
      std::cout << "Fim declaracao";
      break;
    default: break;
  }
}

void Block::printTree(){
  for (Node* line: lines) {
      line->printTree();
      std::cout << std::endl;
  }
}

void BinOp::printTree(){
  auto l = left->type;
  auto r = right->type;

  switch (op) {
    case Ops::assign:
      left->printTree();
      std::cout << ": ";
      right->printTree();
      if(l==Types::real_t && r==Types::integer_t)
        std::cout << " para real";
      break;
    default:
      std::cout << "(";
      left->printTree();
      if(r==Types::real_t && l==Types::integer_t)
        std::cout << " para real";

      std::cout << " (" << Ops::opName[op] << " ";
      if(Ops::masculineOp[op]) std::cout << Types::mascType[type];
      else std::cout << Types::femType[type];
      std::cout << ") ";

      right->printTree();
      if(l==Types::real_t && r==Types::integer_t)
        std::cout << " para real";
      std::cout << ")";
  }
}

void UniOp::printTree(){
  std::cout << "(";
  switch (op) {
    case Ops::u_paren:
      std::cout << "(abre parenteses) ";
      right->printTree();
      std::cout << " (fecha parenteses)";
      break;
    default:
      std::cout << "(" << Ops::opName[op] << " ";
      if(Ops::masculineOp[op]) std::cout << Types::mascType[type];
      else std::cout << Types::femType[type];
      std::cout << ") ";
      right->printTree();
      break;
  }
  std::cout << ")";
}
