#include "st.hpp"
#include "ast.hpp"

using namespace ST;

// other funcs [Symbol]
void Symbol::setType(Types::Type type, std::string compType,
  SymbolTable* typeTable){

  _type = type;
  if(type == Types::composite_t){
    _compType = compType;
    _typeTable = typeTable;
  }
}

Symbol* Symbol::copy(){
  Symbol *s = new Symbol(this->_kind);
  s->_type = this->_type;
  s->_initialized = this->_initialized;
  s->_params = this->_params;
  s->_aSize = this->_aSize;
  s->_compType = this->_compType;
  s->_typeTable = this->_typeTable!=nullptr?
    this->_typeTable->copy():nullptr;

  return s;
}

void Symbol::initializeAllComps(){
  if(_typeTable != nullptr){
    _typeTable->initializeAllVars();
  }
}

// other funcs [SymbolTable]
void SymbolTable::_checkIndex(AST::Node *node){
  auto tmp = AST::Variable::cast(node);
  AST::Array *arr = nullptr;
  while(tmp != nullptr){
    arr = AST::Array::cast(tmp);
    if(arr != nullptr && arr->getIndex() != nullptr &&
        arr->getIndex()->getType() != Types::integer_t){
      Errors::print(Errors::index_wrong_type,
        Types::mascType[arr->getIndex()->getType()]);
    }
    tmp = AST::Variable::cast(tmp->getNextComp());
  }
}

void SymbolTable::assignVariable(AST::Node *node){
  auto tmp = AST::Variable::cast(node);
  tmp->setUse(AST::attr_u);
  Symbol *symbol = getSymbol(tmp->getId());

  this->_checkIndex(node);
  if(symbol != nullptr){
    if(symbol->getKind() != tmp->getKind()){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->getKind()],
        tmp->getId(), Kinds::kindName[tmp->getKind()]);
      tmp->setType(Types::unknown_t, "");
    }else if(tmp->getNextComp() == nullptr){//apenas uma var/array normal
      symbol->setInitialized(true);
      tmp->setType(symbol->getType(), symbol->getCompType());
    }else{//var/array de tipo composto
      tmp->setType(symbol->getType(), symbol->getCompType());
      auto tmp2 = AST::Variable::cast(tmp->getNextComp());
      // percorre os componentes setando seus tipos e uso
      //  e fazendo as devidas verificações
      while(tmp2 != nullptr){
        symbol = symbol->getTypeTable()!=nullptr?
          symbol->getTypeTable()->getSymbol(tmp2->getId()):nullptr;
        if(symbol == nullptr){
          Errors::print(Errors::type_wrong_comp, tmp->getTypeTxt(true),
            tmp2->getId());
          tmp2->setType(Types::unknown_t, "");
        }else{
          tmp2->setType(symbol->getType(), symbol->getCompType());
          if(tmp2->getNextComp() == nullptr)
            symbol->setInitialized(true);
        }
        tmp2->setUse(AST::read_comp_u);
        tmp = tmp2;
        tmp2 = AST::Variable::cast(tmp2->getNextComp());
      }
    }
  }else{
    Errors::print(Errors::without_declaration,
      Kinds::kindName[tmp->getKind()], tmp->getId());
    tmp->setType(Types::unknown_t, "");
  }
}

AST::Node* SymbolTable::declFunction(AST::Node *params, std::string id,
  Types::Type type){

  if ( checkId(id, true) ){
    auto symbol = getSymbol(id);
    if(symbol->getKind() == Kinds::function_t)
      Errors::print(Errors::func_redeclaration, id.c_str());
    else
      Errors::print(Errors::redefinition,
        Kinds::kindName[symbol->getKind()], id.c_str());
  }else{
    Symbol *entry = new Symbol(params, type);
    addSymbol(id, entry);
  }
  return new AST::Function(params, nullptr, id, AST::declr_u,
    "", nullptr, nullptr, type);
}

AST::Node* SymbolTable::defFunction(AST::Node *params, AST::Node *block, std::string id,
  Types::Type type){

  auto p1 = AST::Variable::cast(params);
  auto symbol = getSymbol(id);
  if(symbol != nullptr){//função ja declarada?
    auto p2 = AST::Variable::cast(symbol->getParams());
    bool testKind = symbol->getKind() == Kinds::function_t;
    bool testType = symbol->getType() == type;
    bool testParams = (p2!=nullptr?p2->equals(p1, true):p1==nullptr);

    // Checa redefinição da função, seja por ter alguma parte
    // diferente ou por ter tudo igual a alguma outra função
    // ja declarada/definida
    if( (!testKind || !testType || !testParams) ||
        (testKind && testType && testParams && symbol->isInitialized())){
      Errors:print(Errors::redefinition, Kinds::kindName[symbol->getKind()],
        id.c_str());
      type = Types::unknown_t;
    }else{
      symbol->setInitialized(true);
    }
  }else{//def sem decl
    Symbol *entry = new Symbol(params, type);
    entry->setInitialized(true);
    addSymbol(id, entry);
  }

  return new AST::Function(params, block, id, AST::def_u,
    "", nullptr, nullptr, type);
}

AST::Node* SymbolTable::defCompType(std::string id, AST::Node *block, SymbolTable *st){
  if( checkId(id, true) ){
    auto symbol = getSymbol(id);
    Errors::print(Errors::redefinition,
      Kinds::kindName[symbol->getKind()], id.c_str());
  }else{
    Symbol *entry = new Symbol(Kinds::type_t);
    entry->setTypeTable(st);
    addSymbol(id,entry);
  }

  return new AST::CompositeType(id, block);
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node *next, bool isArray,
  bool insideType/*=false*/){

  Kinds::Kind kind = isArray ? Kinds::array_t : Kinds::variable_t;
  if ( checkId(id, true) ){
    auto symbol = getSymbol(id);
    Errors::print(Errors::redefinition,
      Kinds::kindName[symbol->getKind()], id.c_str());
  }else {
     Symbol *entry = new Symbol(kind);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  if(isArray)
    return new AST::Array(nullptr, 1, id, (insideType ? AST::comp_u : AST::declr_u),
      "", nullptr, next, Types::unknown_t);
  else
    return new AST::Variable(id, (insideType ? AST::comp_u : AST::declr_u),
      "", nullptr, next, Types::unknown_t);
}

/* @param 'useOfFunc'   significa que a variavel esta sendo usada
 *  no uso de uma função. Ex: c := soma(a, b);
 * Isto é uma gambiarra para que possa ser usado arranjos
 *  como parametros de funções
*/
AST::Node* SymbolTable::useVariable(AST::Node *node, bool useOfFunc){
  if(node->getNodeType() == AST::variable_nt)
    node = this->_useVariable(node, useOfFunc);
  else
    node = this->_useArray(node, useOfFunc);

  AST::Variable* tmp1 = AST::Variable::cast(node);
  if(tmp1->getNextComp() != nullptr){
    auto tmp2 = AST::Variable::cast(tmp1->getNextComp());

    if(tmp1->getType() != Types::composite_t){
      Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->getId());
    }else{
      auto symbol = getSymbol(tmp1->getId());
      if(symbol != nullptr){
        symbol = symbol->getTypeTable()!=nullptr?
          symbol->getTypeTable()->getSymbol(tmp2->getId()):nullptr;
        if(symbol == nullptr)
          Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->getId());
      }else{
        Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->getId());
      }
    }
    auto symbol = getSymbol(tmp1->getId());
    if(symbol != nullptr && symbol->getTypeTable()!=nullptr){
      tmp1->setNextComp(symbol->getTypeTable()->useVariable(tmp1->getNextComp(), useOfFunc));
      AST::Variable::cast(tmp1->getNextComp())->setUse(AST::read_comp_u);
    }
  }

  return node;
}

AST::Node* SymbolTable::_useVariable(AST::Node *node, bool useOfFunc){
  auto var = AST::Variable::cast(node);
  var->setUse(AST::read_u);

  if ( ! checkId(var->getId()) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], var->getId());

  auto symbol = getSymbol(var->getId());
  if ( symbol != nullptr ){
    var->setType(symbol->getType(), symbol->getCompType());
    if(symbol->getKind() == Kinds::type_t || symbol->getKind() == Kinds::function_t ||
        ((!useOfFunc||var->getNextComp()!=nullptr) && symbol->getKind() != Kinds::variable_t)){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->getKind()],
        var->getId(), Kinds::kindName[Kinds::variable_t]);
      var->setType(Types::unknown_t, "");
    }else if(symbol->getType() != Types::composite_t && !useOfFunc &&
        checkId(var->getId(),true) && !symbol->isInitialized()){
      Errors::print(Errors::not_initialized, Kinds::kindName[Kinds::variable_t],
        var->getId());
    }
  }else{
    var->setType(Types::unknown_t, "");
  }
  if(useOfFunc && symbol != nullptr && symbol->getKind() == Kinds::array_t
      && var->getNextComp() == nullptr){
    auto arr = new AST::Array(nullptr, symbol->getArraySize(), var->getId(),
      AST::read_u, var->getCompType(), var->getNextComp(), nullptr,
      var->getType());
    delete node;
    return arr;
  }else
    return var;
}

AST::Node* SymbolTable::_useArray(AST::Node *node, bool useOfFunc){
  auto arr = AST::Array::cast(node);
  arr->setUse(AST::read_u);

  if( !checkId(arr->getId()) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::array_t], arr->getId());

  auto symbol = getSymbol(arr->getId());
  if(symbol != nullptr){
    if(symbol->getKind() != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->getKind()],
        arr->getId(), Kinds::kindName[Kinds::array_t]);
    else{
      arr->setType(symbol->getType(), symbol->getCompType());
      arr->setSize(symbol->getArraySize());
    }
  }else{
    arr->setType(Types::unknown_t, "");
  }
  if(arr->getIndex() == nullptr ||
      arr->getIndex()->getType() != Types::integer_t)
    Errors::print(Errors::index_wrong_type,
        Types::mascType[arr->getIndex()->getType()]);

  return arr;
}

AST::Node* SymbolTable::useFunc(AST::Node *params, std::string id){
  if(!checkId(id)) Errors::print(Errors::without_declaration,
      Kinds::kindName[Kinds::function_t], id.c_str());

  //num declared params, num used params
  int nd = 0, nu = 0;
  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  auto uparams = params;
  bool defined = true;

  if(symbol != nullptr){
    // uso errado [18]
    if(symbol->getKind() != Kinds::function_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->getKind()],
        id.c_str(), Kinds::kindName[Kinds::function_t]);
    else
      type = symbol->getType();

    auto dparams = symbol->getParams();
    while(uparams != nullptr){
      if(dparams != nullptr){
        // param de tipo imcompativel [15]
        if(uparams->getType() != dparams->getType())
          Errors::print(Errors::param_wrong_type,Types::mascType[dparams->getType()],
            Types::mascType[uparams->getType()]);
        if(dparams->getNodeType()  == AST::array_nt){
          auto uparams2 = uparams;
          if(uparams->getNodeType() == AST::uniop_nt)//gambiarra para: x := test((v));
            uparams2 = AST::UniOp::cast(uparams2)->getRight(true);
          if(uparams2->getNodeType() != AST::array_nt)
            Errors::print(Errors::param_value_as_array);
          else{
            // tamanho do arranjo tem q ser maior ou igual ao do param [16]
            auto ar = AST::Array::cast(uparams2);
            if(ar->getSize() < AST::Array::cast(dparams)->getSize())
              Errors::print(Errors::array_size_lst_needed, ar->getId());
          }
        }
        ++nd; dparams = dparams->getNext();
      }
      ++nu; uparams = uparams->getNext();
    }
    while(dparams != nullptr){
      ++nd; dparams = dparams->getNext();
    }
  }else{
    while(uparams != nullptr){
      ++nu; uparams = uparams->getNext();
    }
    defined = false;
  }
  // numero de parametros [23]
  if(nd != nu)
    Errors::print(Errors::func_wrong_param_amount,
      id.c_str(), nd, nu);

  // função não declarada deve retornar nó com 0 parametros [25]
  return new AST::Function((defined?params:nullptr), nullptr, id, AST::read_u,
    "", nullptr, nullptr, type);
}

/* Adiciona os parametros da declaração ou definição
 *  de uma função ao escopo do corpo dela
 */
void SymbolTable::addFuncParams(AST::Node *oldParams, AST::Node *newParams){
  AST::Variable *var = nullptr;
  if(oldParams == nullptr)// Função ja foi declarada
    var = AST::Variable::cast(newParams);
  else
    var = AST::Variable::cast(oldParams);

  Symbol *tmp = nullptr;
  while(var != nullptr){
    tmp = getSymbol(var->getCompType());
    Symbol *entry = new Symbol(var->getKind());
    entry->setType(var->getType(), var->getCompType(),
      (tmp!=nullptr?(tmp->getTypeTable()!=nullptr?
        tmp->getTypeTable()->copy():nullptr):nullptr));
    entry->setInitialized(true);
    entry->initializeAllComps();
    if(var->getNodeType() == AST::array_nt)
      entry->setArraySize(AST::Array::cast(var)->getSize());

    addSymbol(var->getId(), entry);
    var = AST::Variable::cast(var->getNext());
  }
}

void SymbolTable::checkFuncs(){
  for(const auto& iter : _entryList){
    const auto& symbol = iter.second;
    if(symbol->getKind() == Kinds::function_t && !symbol->isInitialized())
      Errors::print(Errors::func_never_declared, iter.first.c_str());
  }
}

void SymbolTable::initializeAllVars(){
  for(auto& iter : _entryList){
    auto& symbol = iter.second;
    symbol->setInitialized(true);
  }
}

void SymbolTable::setType(AST::Node *node, Types::Type type, std::string compType){
  auto var = AST::Variable::cast(node);
  Symbol *varSymbol = nullptr;
  Symbol *ctSymbol = getSymbol(compType);

  if(type==Types::composite_t && ctSymbol==nullptr){
    Errors::print(Errors::type_undefined, compType.c_str());
  }else{
    while(var != nullptr){
      varSymbol = getSymbol(var->getId());
      if(varSymbol != nullptr && (varSymbol->getKind() == Kinds::variable_t ||
          varSymbol->getKind() == Kinds::array_t) &&
          varSymbol->getType() == Types::unknown_t){
        // Cada variavel tem que ter uma cópia das variaveis
        //  do corpo do tipo [tabela de simbolos do tipo]
        varSymbol->setType(type, compType, ctSymbol!=nullptr?
          ctSymbol->getTypeTable()->copy():nullptr);
        var->setType(type, compType);
      }
      var = AST::Variable::cast(var->getNext());
    }
  }
}

void SymbolTable::setArraySize(AST::Node *node, int size){
  auto arr = AST::Array::cast(node);
  Symbol *tmp = nullptr;
  while(arr != nullptr){
    arr->setSize(size);
    tmp = getSymbol(arr->getId());
    if(tmp->getKind() == Kinds::array_t &&
        tmp->getArraySize() == -1)
      tmp->setArraySize(size);

    arr = AST::Array::cast(arr->getNext());
  }
}

/* @param creatin     É declaração de variavel?
 */
bool SymbolTable::checkId(std::string id, bool creation/*=false*/){
  bool result = _entryList.find(id) != _entryList.end();
  // Caso seja a declaração de uma variavel
  // só é preciso checar o primeiro nivel
  // do escopo
  if(creation)
    return result;
  return result ? result :
    (_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

void SymbolTable::addSymbol(std::string id, Symbol *newsymbol){
  _entryList[id] = newsymbol;
}

SymbolTable* SymbolTable::copy(){
  SymbolTable *st = new SymbolTable(this->_previous);
  for(auto& iter : _entryList){
    auto& symbol = iter.second;
    st->addSymbol(iter.first, symbol->copy());
  }
  return st;
}

// getters [SymbolTable]
Symbol* SymbolTable::getSymbol(std::string id){
  bool result = _entryList.find(id) != _entryList.end();
  if(result)
    return _entryList[id];
  else if(_previous != nullptr)
    return _previous->getSymbol(id);
  else
    return nullptr;
}
