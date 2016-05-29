#include "ast.hpp"

using namespace AST;

// Construtores

CondExpr::CondExpr(Node *cond, Node *thenBranch, Node *elseBranch):
  cond(cond), thenBranch(thenBranch), elseBranch(elseBranch){

  if(cond->type != Types::bool_t){
    Errors::print(Errors::op_wrong_type1, "enquanto",
        Types::mascType[Types::bool_t], Types::mascType[cond->type]);
  }
}

WhileExpr::WhileExpr(Node *cond, Node *block):
  cond(cond), block(block){

  if(cond->type != Types::bool_t){
    Errors::print(Errors::op_wrong_type1, "teste",
        Types::mascType[Types::bool_t], Types::mascType[cond->type]);
  }
}

Function::Function(std::string id, Node *params, Node *block, Use use,
  Types::Type type/*=Types::unknown_t*/):
  params(params), block(block), Variable(id,nullptr,use,type){

  // Verifica se tem pelo menos um return no corpo
  // da função e verifica se o tipo do return
  // bate com o da função
  if(block != nullptr){
    bool foundReturn = false;
    Block *b = (Block*)block;
    Return *r = nullptr;
    for(Node *line : b->lines){
      if(line->getNodeType() == return_nt){
        r = (Return*)line;
        r->funcType = type;//gambiarra para coerção
        if(r->type!=type && !(type==Types::real_t && r->type==Types::integer_t))
          Errors::print(Errors::op_wrong_type1, "retorno",
            Types::mascType[type], Types::mascType[line->type]);

        foundReturn = true;
      }
    }

    if(!foundReturn)
      Errors::print(Errors::func_without_return);
  }
}

Array::Array(std::string id, Node *next, Node *i,
  Use use, int aSize, std::string compType, Types::Type type):
  Variable(id,next,use,compType,type){

  // Verifica se o tamanho do arranjo é
  // maior ou igual a 1
  if(aSize < 1){
    aSize = 1;
    Errors::print(Errors::array_size_lst_1, id.c_str());
  }
  // Verifica se o index é uma expressão inteira
  if(i != nullptr && i->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[i->type]);

  size = aSize;
  index = i;
}

BinOp::BinOp(Node *left, Ops::Operation op, Node *right):
  left(left), op(op), right(right){

  // verificações se o lado esquerdo ou direito
  // da operação é um arranjo, gambiarra por causa
  // dos parametros para uso de funções...
  if(op != Ops::assign){
    if(left->getNodeType() == array_nt){
      auto tmp = dynamic_cast<Array*>(left);
      if(tmp->index == nullptr){
        left->type = Types::unknown_t;
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->id.c_str(), Kinds::kindName[Kinds::variable_t]);
      }
    }
    if(right->getNodeType() == array_nt){
      auto tmp = dynamic_cast<Array*>(right);
      if(tmp->index == nullptr){
        right->type = Types::unknown_t;
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->id.c_str(), Kinds::kindName[Kinds::variable_t]);
      }
    }
  }

  // Gambiarra para pegar o tipo do ultimo
  //  elemento da atribuição
  // Ex: p[1].x := 5.0;
  auto ltmp = left;
  auto rtmp = right;
  while(ltmp->next != nullptr)
    ltmp = ltmp->next;
  while(rtmp->next != nullptr)
    rtmp = rtmp->next;

  // Tipo do lado esquerdo
  auto l = ltmp->type;
  // Texto do tipo do lado esquerdo
  auto ltxt = Types::mascType[l];
  // Tipo do lado direito
  auto r = rtmp->type;
  // Texto do tipo do lado direito
  auto rtxt = Types::mascType[r];
  // Texto da operação
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
      // Coerção
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

  // verificações se o lado direito
  // da operação é um arranjo, gambiarra por causa
  // dos parametros para uso de funções...
  if(op != Ops::u_paren){
    if(right->getNodeType() == array_nt){
      auto tmp = dynamic_cast<Array*>(right);
      if(tmp->index == nullptr){
        right->type = Types::unknown_t;
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->id.c_str(), Kinds::kindName[Kinds::variable_t]);
      }
    }
  }

  // Tipo do lado direito
  auto r = right->type;
  // Texto do tipo do lado direito
  auto rtxt = Types::mascType[r];
  // Texto da operação
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

// Prints

void Value::printTree(){
  std::cout << "valor " << Types::mascType[type] << " " << n;
  if(next != nullptr){
    std::cout << ", ";
    next->printTree();
  }
}

void Variable::printTree(){
  switch (use) {
    case declr:{
      std::cout << "Declaracao de variavel " << getTypeTxt(false) <<
        ": " << id;
      break;
    }case attr:{
      std::cout << "Atribuicao de valor para variavel " <<
        getTypeTxt(false)   << " " << id;
      break;
    }case read:{
      std::cout << "variavel " << getTypeTxt(false)  << " " << id;
      break;
    }case read_comp:{
        std::cout << " componente " << getTypeTxt(true)  << " "
          << id;
        break;
    }case param:{
      std::cout << "Parametro " << getTypeTxt(true)  << ": "
        << id << "\n";
      break;
    }case comp:{
      std::cout << "Componente " << getTypeTxt(true)  << ": "
        << id;
      break;
    }default: break;
  }
  if(next != NULL){
    if(use==declr || use==comp){
      auto tmp = dynamic_cast<Variable*>(next);
      while(tmp != nullptr){
        std::cout << ", " << tmp->id;
        tmp = dynamic_cast<Variable*>(tmp->next);
      }
    }else{
      std::cout << (use==read?", ":"");
      next->printTree();
    }
  }
}

void Array::printTree(){
  switch (use) {
    case declr:{
      std::cout << "Declaracao de arranjo " << getTypeTxt(true)
        << " de tamanho " << size << ": " << id;
        break;
    }case attr:{
      std::cout << "Atribuicao de valor para arranjo " <<
        getTypeTxt(true)  << " " << id << " {+indice: ";
      index->printTree();
      std::cout << "}";
      break;
    }case read:{
      std::cout << "arranjo " << getTypeTxt(true) << " " << id;
      if(index != nullptr){
        std::cout << " {+indice: ";
        index->printTree();
        std::cout << "}";
      }
      break;
    }case read_comp:{
        std::cout << " componente " << getTypeTxt(true)  << " "
          << id;
		if(index != nullptr){
		  std::cout << " {+indice: ";
		  index->printTree();
		  std::cout << "}";
	    }
        break;
    }case param:{
      std::cout << "Parametro arranjo " << getTypeTxt(true) << " de tamanho " <<
        size << ": " << id << "\n";
      break;
    }case comp:{
      std::cout << "Componente arranjo " << getTypeTxt(true) << " de tamanho " <<
        size << ": " << id;
      break;
    }default: break;
  }
  if(next != NULL){
    if(use==declr || use==comp){
      auto tmp = dynamic_cast<Variable*>(next);
      while(tmp != nullptr){
        std::cout << ", " << tmp->id;
        tmp = dynamic_cast<Variable*>(tmp->next);
      }
    }else{
      std::cout << (use==read?", ":"");
      next->printTree();
    }
  }
}

void Function::printTree(){
  switch (use) {
    case declr:{
      std::cout << "Declaracao de funcao " << Types::femType[type] <<
        ": " << id << "\n+parametros:\n";
      if(params!=nullptr)params->printTree();
      std::cout << "Fim declaracao";
      break;
    }case def:{
      std::cout << "Definicao de funcao " << Types::femType[type] <<
        ": " << id << "\n+parametros:\n";
      if(params!=nullptr)params->printTree();
      std::cout << "+corpo:\n";
      block->printTree();
      std::cout << "Fim definicao";
      break;
    }case read:{
      std::cout << "chamada de funcao " << Types::femType[type] <<
        " " << id.c_str() << " {+parametros: ";

      if(params != nullptr)
        params->printTree();
      std::cout << "}";
      break;
    }default: break;
  }
}

void Return::printTree(){
  std::cout << "Retorno de funcao: ";
  expr->printTree();
  // Coerção
  if(funcType==Types::real_t && type==Types::integer_t)
    std::cout << " para real";
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
      // Coerção
      if(l==Types::real_t && r==Types::integer_t)
        std::cout << " para real";
      break;
    default:
      std::cout << "(";
      left->printTree();
      // Coerção
      if(r==Types::real_t && l==Types::integer_t)
        std::cout << " para real";

      std::cout << " (" << Ops::opName[op] << " ";
      if(Ops::masculineOp[op]) std::cout << Types::mascType[type];
      else std::cout << Types::femType[type];
      std::cout << ") ";

      right->printTree();
      // Coerção
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

void CondExpr::printTree(){
  std::cout << "Expressao condicional\n+se: ";
  cond->printTree();
  std::cout << "\n+entao: \n";
  thenBranch->printTree();
  if(elseBranch != nullptr){
    std::cout << "+senao: \n";
    elseBranch->printTree();
  }
  std::cout << "Fim expressao condicional";
}

void WhileExpr::printTree(){
  std::cout << "Laco\n+enquanto: ";
  cond->printTree();
  std::cout << "\n+faca:\n";
  block->printTree();
  std::cout << "Fim laco";
}

void CompositeType::printTree(){
  std::cout << "Definicao tipo: " << id <<
    "\n+componentes: \n";
  block->printTree();
  std::cout << "Fim definicao";
}

// Outras funções

bool Variable::equals(Variable *var, bool checkNext/*=false*/){
  if(var == nullptr || var->getKind() != this->getKind())
    return false;

  auto next1 = (Variable*)next;
  auto next2 = (Variable*)var->next;
  return this->id==var->id && this->type==var->type &&
    (checkNext ? (next1!=nullptr?next1->equals(next2):next2==nullptr) : true);
}

bool Array::equals(Variable *var, bool checkNext/*=false*/){
  bool ret = Variable::equals(var, checkNext);

  auto ar = dynamic_cast<Array*>(var);
  return ret && this->size==ar->size;
}

bool Function::equals(Variable *var, bool checkNext/*=false*/){
  bool ret = Variable::equals(var, checkNext);

  auto func = dynamic_cast<Function*>(var);
  auto params1 = (Variable*)params;
  auto params2 = (Variable*)func->params;
  return ret && (params1!=nullptr?params1->equals(params2, true):params2==nullptr);
}

void Array::setSize(int n){
  if(n < 1){
    n = 1;
    Errors::print(Errors::array_size_lst_1, id.c_str());
  }
  size = n;
}

void Variable::setType(Types::Type t, std::string tId){
  type = t;
  if(t == Types::composite_t)
    compType = tId;
}

const char* Variable::getTypeTxt(bool masc){
  if(type!=Types::composite_t)
    return masc ? Types::mascType[type] : Types::femType[type];
  else
    return compType.c_str();
}
