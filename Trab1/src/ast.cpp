#include "ast.hpp"

using namespace AST;

// constructors
Array::Array(Node *index, int size, std::string id, Use use,
  std::string compType, Node *nextComp, Node *next,
  Types::Type type/*=Types::unknown_t*/):
  _index(index), _size(size),
  Variable(id,use,compType,nextComp,next,type){

  // Verifica se o tamanho do arranjo é
  // maior ou igual a 1
  if(_size < 1){
    _size = 1;
    Errors::print(Errors::array_size_lst_1, getId());
  }

  // Verifica se o index é uma expressão inteira
  if(_index != nullptr && _index->getType() != Types::integer_t)
    Errors::print(Errors::index_wrong_type,
      Types::mascType[_index->getType()]);
}

Function::Function(Node *params, Node *block, std::string id, Use use,
  std::string compType, Node *nextComp, Node *next,
  Types::Type type/*=Types::unknown_t*/):
  _params(params), _block(block),
  Variable(id,use,compType,nextComp,next,type){

  // Verifica se tem pelo menos um return no corpo
  // da função e verifica se o tipo do return
  // bate com o da função
  Types::Type ftype = getType(), rtype = Types::unknown_t;
  if(_block != nullptr){
    bool found = false;
    Block *b = Block::cast(_block);
    Return *r = nullptr;
    for(Node *line : b->getLines()){
      r = Return::cast(line);
      if(r != nullptr){
        r->setFuncType(ftype);
        rtype = r->getType();
        if(rtype != ftype &&
            !(ftype==Types::real_t && rtype==Types::integer_t))
          Errors::print(Errors::op_wrong_type1, "retorno",
            Types::mascType[ftype], Types::mascType[rtype]);

        found = true;
      }
    }
    if(!found)
      Errors::print(Errors::func_without_return);
  }
}

CondExpr::CondExpr(Node *cond, Node *thenBranch, Node *elseBranch):
  _cond(cond), _thenBranch(thenBranch), _elseBranch(elseBranch){

  if(_cond->getType() != Types::bool_t){
    Errors::print(Errors::op_wrong_type1, "enquanto",
        Types::mascType[Types::bool_t], Types::mascType[_cond->getType()]);
  }
}

WhileExpr::WhileExpr(Node *cond, Node *block):
  _cond(cond), _block(block){

  if(_cond->getType() != Types::bool_t){
    Errors::print(Errors::op_wrong_type1, "teste",
        Types::mascType[Types::bool_t], Types::mascType[_cond->getType()]);
  }
}

BinOp::BinOp(Node *left, Ops::Operation op, Node *right):
  _left(left), _op(op), _right(right){

  // verificações se o lado esquerdo ou direito
  // da operação é um arranjo, gambiarra por causa
  // dos parametros para uso de funções...
  if(_op != Ops::assign){
    Array* tmp = Array::cast(_left);
    if(tmp != nullptr){
      if(tmp->getIndex() == nullptr){
        _left->setType(Types::unknown_t);
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->getId(), Kinds::kindName[Kinds::variable_t]);
      }
    }
    tmp = Array::cast(_right);
    if(tmp != nullptr){
      if(tmp->getIndex() == nullptr){
        _right->setType(Types::unknown_t);
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->getId(), Kinds::kindName[Kinds::variable_t]);
      }
    }
  }

  // Tipo do lado esquerdo
  auto l = _left->getType(true);
  // Texto do tipo do lado esquerdo
  auto ltxt = Types::mascType[l];
  // Tipo do lado direito
  auto r = _right->getType(true);
  // Texto do tipo do lado direito
  auto rtxt = Types::mascType[r];
  // Texto da operação
  auto optxt = Ops::opName[_op];

  switch (_op) {
    case Ops::assign:{
      setType(l);
      if(l != r && !(l==Types::real_t && r==Types::integer_t))
        Errors::print(Errors::op_wrong_type1, optxt, ltxt, rtxt);
      break;
    }
    case Ops::plus:
    case Ops::b_minus:
    case Ops::times:
    case Ops::division:{
      setType(Types::integer_t);
      // Coerção
      if( (l==Types::real_t && r==Types::real_t) ||
          (l==Types::real_t && r==Types::integer_t) ||
          (l==Types::integer_t && r==Types::real_t)){
        setType(Types::real_t);
      }
      if(l!=Types::integer_t && l!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, ltxt);
      if(r!=Types::integer_t && r!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;
    }
    case Ops::eq:
    case Ops::neq:
    case Ops::grt:
    case Ops::grteq:
    case Ops::lst:
    case Ops::lsteq:{
      setType(Types::bool_t);
      if(l!=Types::integer_t && l!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, ltxt);
      if(r!=Types::integer_t && r!=Types::real_t)
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;
    }
    case Ops::b_and:
    case Ops::b_or:{
      setType(Types::bool_t);
      if(l!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], ltxt);
      if(r!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], rtxt);
      break;
    }
    default: setType(Types::unknown_t);
  }
}

UniOp::UniOp(Ops::Operation op, Node *right):
  _op(op), _right(right){

  // verificações se o lado direito
  // da operação é um arranjo, gambiarra por causa
  // dos parametros para uso de funções...
  if(_op != Ops::u_paren){
    auto tmp = Array::cast(_right);
    if(tmp != nullptr){
      if(tmp->getIndex() == nullptr){
        _right->setType(Types::unknown_t);
        Errors::print(Errors::wrong_use, Kinds::kindName[Kinds::array_t],
          tmp->getId(), Kinds::kindName[Kinds::variable_t]);
      }
    }
  }

  // Tipo do lado direito
  auto r = _right->getType(true);
  // Texto do tipo do lado direito
  auto rtxt = Types::mascType[r];
  // Texto da operação
  auto optxt = Ops::opName[_op];

  switch (_op) {
    case Ops::u_not:{
      setType(Types::bool_t);
      if(r!=Types::bool_t)
        Errors::print(Errors::op_wrong_type1, optxt,
          Types::mascType[Types::bool_t], rtxt);
      break;
    }
    case Ops::u_minus:{
      setType(Types::integer_t);
      if(r==Types::real_t || r==Types::integer_t)
        setType(r);
      else
        Errors::print(Errors::op_wrong_type2, optxt, rtxt);
      break;
    }
    case Ops::u_paren:{
      setType(r);
      break;
    }
    default: setType(Types::unknown_t);
  }
}

// destructors
Node::~Node(){
  if(_next != nullptr)
    delete _next;
}

Block::~Block(){
  for(auto& line : _lines)
    delete line;
}

Array::~Array(){
  if(_index != nullptr)
    delete _index;
}

Function::~Function(){
  if(_params != nullptr)
    delete _params;
  if(_block != nullptr)
    delete _block;
}

Return::~Return(){
  if(_expr != nullptr)
    delete _expr;
}

CondExpr::~CondExpr(){
  if(_cond != nullptr)
    delete _cond;
  if(_thenBranch != nullptr)
    delete _thenBranch;
  if(_elseBranch != nullptr)
    delete _elseBranch;
}

WhileExpr::~WhileExpr(){
  if(_cond != nullptr)
    delete _cond;
  if(_block != nullptr)
    delete _block;
}

BinOp::~BinOp(){
  if(_left != nullptr)
    delete _left;
  if(_right != nullptr)
    delete _right;
}

UniOp::~UniOp(){
  if(_right != nullptr)
    delete _right;
}

// getters
/* 'checkComps' só usado na classe 'Variable'
 */
Types::Type Node::getType(bool checkComps/*=false*/){
  return _type;
}

const char* Node::getTypeTxt(bool masc){
  if(masc)
    return Types::mascType[_type];
  else
    return Types::femType[_type];
}

/* @param checkComps    É para pegar o tipo do ultimo componente?
 */
Types::Type Variable::getType(bool checkComps/*=false*/){
  if(!checkComps)
    return Node::getType();

  auto comp = Variable::cast(_nextComp);
  return comp != nullptr ? comp->getType(checkComps) : Node::getType();
}

const char* Variable::getTypeTxt(bool masc){
  if(hasCompType())
    return _compType.c_str();
  else
    return Node::getTypeTxt(masc);
}

/* @param checkParens     É para pegar a expr dentro do parenteses?
 */
Node* BinOp::getLeft(bool checkParens/*=false*/){
  if(!checkParens)
    return _left;

  auto tmp = UniOp::cast(_left);
  if(tmp != nullptr && tmp->getOp() == Ops::u_paren)
    return tmp->getRight(checkParens);

  return _left;
}

/* @param checkParens     É para pegar a expr dentro do parenteses?
 */
Node* BinOp::getRight(bool checkParens/*=false*/){
  if(!checkParens)
    return _right;

  auto tmp = UniOp::cast(_right);
  if(tmp != nullptr && tmp->getOp() == Ops::u_paren)
    return tmp->getRight(checkParens);

  return _right;
}

/* @param checkParens     É para pegar a expr dentro do parenteses?
 */
Node* UniOp::getRight(bool checkParens/*=false*/){
  if(!checkParens)
    return _right;

  auto tmp = UniOp::cast(_right);
  if(tmp != nullptr && tmp->getOp() == Ops::u_paren)
    return tmp->getRight(checkParens);

  return _right;
}



// setters
void Array::setSize(int size){
  // verifica se o tamanho é maior ou igual a 1
  if(size < 1){
    size = 1;
    Errors::print(Errors::array_size_lst_1, getId());
  }
  _size = size;
}

void Variable::setType(Types::Type type, std::string compType){
  Node::setType(type);
  if(type == Types::composite_t)
    setCompType(compType);
}

// printTrees
void Block::printTree(){
  for(auto& line : _lines) {
      line->printTree();
      std::cout << std::endl;
  }
}

void Value::printTree(){
  std::cout << "valor " << getTypeTxt(true) << " " << getN();
  if(getNext() != nullptr){//chamada de função
    std::cout << ", ";
    getNext()->printTree();
  }
}

void Variable::printTree(){
  switch (_use) {
    case declr_u:{
      std::cout << "Declaracao de variavel " << getTypeTxt(false) <<
        ": " << getId();
      break;
    }
    case attr_u:{
      std::cout << "Atribuicao de valor para variavel " <<
        getTypeTxt(false) << " " << getId();
      break;
    }
    case read_u:{
      std::cout << "variavel " << getTypeTxt(false) << " " << getId();
      break;
    }
    case read_comp_u:{
      std::cout << " componente " << getTypeTxt(true) << " "
        << getId();
      break;
    }
    case param_u:{
      std::cout << "Parametro " << getTypeTxt(true) << ": "
        << getId() << "\n";
      break;
    }
    case comp_u:{
      std::cout << "Componente " << getTypeTxt(true) << ": "
        << getId();
      break;
    }
    default: break;
  }

  if(getNextComp() != nullptr)
    getNextComp()->printTree();

  auto next = getNext();
  if(next != nullptr){
    if(_use == declr_u || _use == comp_u){
      auto tmp = Variable::cast(next);
      while(tmp != nullptr){
        std::cout << ", " << tmp->getId();
        tmp = Variable::cast(tmp->getNext());
      }
    }else{
      std::cout << (_use==read_u?", ":"");
      next->printTree();
    }
  }
}

void Array::printTree(){
  switch (_use) {
    case declr_u:{
      std::cout << "Declaracao de arranjo " << getTypeTxt(true)
        << " de tamanho " << getSize() << ": " << getId();
      break;
    }
    case attr_u:{
      std::cout << "Atribuicao de valor para arranjo " <<
        getTypeTxt(true)  << " " << getId() << " {+indice: ";
      _index->printTree();
      std::cout << "}";
      break;
    }
    case read_u:{
      std::cout << "arranjo " << getTypeTxt(true) << " " << getId();
      if(_index != nullptr){
        std::cout << " {+indice: ";
        _index->printTree();
        std::cout << "}";
      }
      break;
    }
    case read_comp_u:{
      std::cout << " componente arranjo " << getTypeTxt(true) << " "
          << getId();
  		if(_index != nullptr){
  		  std::cout << " {+indice: ";
  		  _index->printTree();
  		  std::cout << "}";
  	  }
      break;
    }
    case param_u:{
      std::cout << "Parametro arranjo " << getTypeTxt(true) << " de tamanho " <<
        getSize() << ": " << getId() << "\n";
      break;
    }
    case comp_u:{
      std::cout << "Componente arranjo " << getTypeTxt(true) << " de tamanho " <<
        getSize() << ": " << getId();
      break;
    }
    default: break;
  }

  if(getNextComp() != nullptr)
    getNextComp()->printTree();

  auto next = getNext();
  if(next != nullptr){
    if(_use == declr_u || _use == comp_u){
      auto tmp = Variable::cast(next);
      while(tmp != nullptr){
        std::cout << ", " << tmp->getId();
        tmp = Variable::cast(tmp->getNext());
      }
    }else{
      std::cout << (_use==read_u?", ":"");
      next->printTree();
    }
  }
}

void Function::printTree(){
  switch (_use) {
    case declr_u:{
      std::cout << "Declaracao de funcao " << getTypeTxt(false) <<
        ": " << getId() << "\n+parametros:\n";
      if(_params!=nullptr) _params->printTree();
      std::cout << "Fim declaracao";
      break;
    }
    case def_u:{
      std::cout << "Definicao de funcao " << getTypeTxt(false) <<
        ": " << getId() << "\n+parametros:\n";
      if(_params!=nullptr) _params->printTree();
      std::cout << "+corpo:\n";
      if(_block!=nullptr) _block->printTree();
      std::cout << "Fim definicao";
      break;
    }
    case read_u:{
      std::cout << "chamada de funcao " << getTypeTxt(false) <<
        " " << getId() << " {+parametros: ";
      if(_params != nullptr) _params->printTree();
      std::cout << "}";
      break;
    }
    default: break;
  }
}

void CompositeType::printTree(){
  std::cout << "Definicao tipo: " << getId() <<
    "\n+componentes: \n";
  _block->printTree();
  std::cout << "Fim definicao";
}

void Return::printTree(){
  std::cout << "Retorno de funcao: ";
  _expr->printTree();
  // Coerção
  if(_funcType==Types::real_t && getType()==Types::integer_t)
    std::cout << " para real";
}

void CondExpr::printTree(){
  std::cout << "Expressao condicional\n+se: ";
  _cond->printTree();
  std::cout << "\n+entao: \n";
  _thenBranch->printTree();
  if(_elseBranch != nullptr){
    std::cout << "+senao: \n";
    _elseBranch->printTree();
  }
  std::cout << "Fim expressao condicional";
}

void WhileExpr::printTree(){
  std::cout << "Laco\n+enquanto: ";
  _cond->printTree();
  std::cout << "\n+faca:\n";
  _block->printTree();
  std::cout << "Fim laco";
}

void BinOp::printTree(){
  auto l = _left->getType(true);
  auto r = _right->getType(true);

  switch (_op) {
    case Ops::assign:{
      _left->printTree();
      std::cout << ": ";
      _right->printTree();
      // Coerção
      if(l==Types::real_t && r==Types::integer_t)
        std::cout << " para real";
      break;
    }
    default:{
      std::cout << "(";
      _left->printTree();
      // Coerção
      if(r==Types::real_t && l==Types::integer_t)
        std::cout << " para real";

      std::cout << " (" << Ops::opName[_op] << " ";
      if(Ops::masculineOp[_op]) std::cout << Types::mascType[getType()];
      else std::cout << Types::femType[getType()];
      std::cout << ") ";

      _right->printTree();
      // Coerção
      if(l==Types::real_t && r==Types::integer_t)
        std::cout << " para real";
      std::cout << ")";
      break;
    }
  }
}

void UniOp::printTree(){
  std::cout << "(";
  switch (_op) {
    case Ops::u_paren:{
      std::cout << "(abre parenteses) ";
      _right->printTree();
      std::cout << " (fecha parenteses)";
      break;
    }
    default:{
      std::cout << "(" << Ops::opName[_op] << " ";
      if(Ops::masculineOp[_op]) std::cout << Types::mascType[getType()];
      else std::cout << Types::femType[getType()];
      std::cout << ") ";
      _right->printTree();
      break;
    }
  }
  std::cout << ")";
}

// other funcs
void Block::addLine(Node *line){
  _lines.push_back(line);
}

bool Variable::hasCompType(){
  return getType() == Types::composite_t;
}

/* @param checkNext     Checar se as variaveis 'next' são iguais tambem?
 */
bool Variable::equals(Variable *var, bool checkNext/*=false*/){
  if(var == nullptr || var->getKind() != this->getKind())
    return false;

  auto n1 = Variable::cast(this->getNext());
  auto n2 = Variable::cast(var->getNext());
  return this->_id==var->_id && this->getType()==var->getType() &&
    (checkNext ? (n1!=nullptr?n1->equals(n2,checkNext):n2==nullptr) : true);
}

/* @param checkNext     Checar se as variaveis 'next' são iguais tambem?
 */
bool Array::equals(Variable *var, bool checkNext/*=false*/){
  bool ret = Variable::equals(var, checkNext);

  auto ar = Array::cast(var);
  return ret && this->getSize()==ar->getSize();
}

/* @param checkNext     Checar se as variaveis 'next' são iguais tambem?
 */
bool Function::equals(Variable *var, bool checkNext/*=false*/){
  bool ret = Variable::equals(var, checkNext);

  auto func = Function::cast(var);
  auto p1 = Variable::cast(this->_params);
  auto p2 = Variable::cast(func->_params);
  return ret && (p1!=nullptr?p1->equals(p2,true):p2==nullptr);
}

// static funcs
Block* Block::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == block_nt)
    return dynamic_cast<Block*>(node);
  else
    return nullptr;
}

Value* Value::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == value_nt)
    return dynamic_cast<Value*>(node);
  else
    return nullptr;
}

Variable* Variable::cast(Node *node){
  if(node != nullptr){
    auto type = node->getNodeType();
    if(type==variable_nt||type==array_nt||type==function_nt)
      return dynamic_cast<Variable*>(node);
    else
      return nullptr;
  }else
    return nullptr;
}

Array* Array::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == array_nt)
    return dynamic_cast<Array*>(node);
  else
    return nullptr;
}

Function* Function::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == function_nt)
    return dynamic_cast<Function*>(node);
  else
    return nullptr;
}

CompositeType* CompositeType::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == composite_type_nt)
    return dynamic_cast<CompositeType*>(node);
  else
    return nullptr;
}

Return* Return::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == return_nt)
    return dynamic_cast<Return*>(node);
  else
    return nullptr;
}

CondExpr* CondExpr::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == condexpr_nt)
    return dynamic_cast<CondExpr*>(node);
  else
    return nullptr;
}

WhileExpr* WhileExpr::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == whileexpr_nt)
    return dynamic_cast<WhileExpr*>(node);
  else
    return nullptr;
}

BinOp* BinOp::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == binop_nt)
    return dynamic_cast<BinOp*>(node);
  else
    return nullptr;
}

UniOp* UniOp::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == uniop_nt)
    return dynamic_cast<UniOp*>(node);
  else
    return nullptr;
}
