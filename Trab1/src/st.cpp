#include "st.hpp"
#include "ast.hpp"

using namespace ST;

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

/* Procura o id passado em todos os escopos e
    retorno o simbolo deste id. */
Symbol* SymbolTable::getSymbol(std::string id){
  bool result = _entryList.find(id) != _entryList.end();
  if(result)
    return _entryList[id];
  else if(_previous != nullptr)
    return _previous->getSymbol(id);
  else
    return nullptr;
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next, bool isArray,
  bool insideType/*=false*/){

  Kinds::Kind kind = isArray ? Kinds::array_t : Kinds::variable_t;
  if ( checkId(id, true) ){
    auto symbol = getSymbol(id);
    Errors::print(Errors::redefinition,
      Kinds::kindName[symbol->kind], id.c_str());
  }else {
     Symbol *entry = new Symbol(kind);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  if(isArray)
    return new AST::Array(id, next, (insideType ? AST::comp : AST::declr));
  else
    return new AST::Variable(id, next, (insideType ? AST::comp : AST::declr));
}

AST::Node* SymbolTable::declFunction(std::string id, AST::Node *params, Types::Type type){
  if ( checkId(id, true) ){
    Errors::print(Errors::func_redeclaration, id.c_str());
  }else {
    Symbol *entry = new Symbol(params, type);
    addSymbol(id, entry);
  }
  return new AST::Function(id, params, nullptr, AST::declr, type);
}

AST::Node* SymbolTable::defFunction(std::string id, AST::Node *params,
  AST::Node *block, Types::Type type){

  auto params1 = (AST::Variable*)params;
  auto symbol = getSymbol(id);
  if(symbol != nullptr){
    auto params2 = (AST::Variable*)symbol->params;
    bool testKind = symbol->kind == Kinds::function_t;
    bool testType = symbol->type == type;
    bool testParams = (params2!=nullptr?params2->equals(params1, true):params1==nullptr);

    // Checa redefinição da função, seja por ter alguma parte
    // diferente ou por ter tudo igual a alguma outra função
    // ja declarada/definida
    if( (!testKind || !testType || !testParams) ||
        (testKind && testType && testParams && symbol->initialized)){
      Errors:print(Errors::redefinition, Kinds::kindName[symbol->kind],
        id.c_str());
      type = Types::unknown_t;
    }else{
      symbol->initialized = true;
    }
  }else{
    Symbol *entry = new Symbol(params, type);
    entry->initialized = true;
    addSymbol(id, entry);
  }

  return new AST::Function(id, params, block, AST::def, type);
}

AST::Node* SymbolTable::defCompType(std::string id, AST::Node *block){
  if( checkId(id, true) ){
    Errors::print(Errors::redefinition,
      Kinds::kindName[Kinds::type_t], id.c_str());
  }else{
    Symbol *entry = new Symbol(Kinds::type_t);
    entry->typeTable = dynamic_cast<AST::Block*>(block)->symtab;
    addSymbol(id,entry);
  }

  return new AST::CompositeType(id, block);
}

void SymbolTable::assignVariable(AST::Node *var){
  AST::Variable *tmp = dynamic_cast<AST::Variable*>(var);
  tmp->use = AST::attr;
  Symbol *symbol = getSymbol(tmp->id);

  if(symbol != nullptr){
    if(symbol->kind != tmp->getKind()){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind], tmp->id.c_str(),
        Kinds::kindName[tmp->getKind()]);
    }else{
      if(var->next == nullptr){//apenas uma var/array normal
        symbol->initialized = true;
        tmp->type = symbol->type;
        tmp->compType = symbol->compTypeId;
      }else{//var/array com tipo composto
        tmp->type = symbol->type;
        tmp->compType = symbol->compTypeId;
        auto tmp2 = dynamic_cast<AST::Variable*>(tmp->next);
        while(tmp2 != nullptr){
          symbol = symbol->typeTable!=nullptr?
            symbol->typeTable->getSymbol(tmp2->id):nullptr;
          if(symbol == nullptr){
            Errors::print(Errors::type_wrong_comp, tmp->getTypeTxt(true),
              tmp2->id.c_str());
          }else{
            tmp2->type = symbol->type;
            tmp2->compType = symbol->compTypeId;
            if(tmp2->next == nullptr)
              symbol->initialized = true;
          }

          tmp2->use = AST::read_comp;
          tmp = tmp2;
          tmp2 = dynamic_cast<AST::Variable*>(tmp2->next);
        }
      }
    }
  }else{
    Errors::print(Errors::without_declaration,
      Kinds::kindName[tmp->getKind()], tmp->id.c_str());
  }
}

AST::Node* SymbolTable::useVariable(AST::Node *node, bool useOfFunc){
  if(node->getNodeType() == AST::variable_nt)
    node = this->_useVariable(node, useOfFunc);
  else
    node = this->_useArray(node, useOfFunc);

  if(node->next != nullptr){
    auto tmp1 = dynamic_cast<AST::Variable*>(node);
    auto tmp2 = dynamic_cast<AST::Variable*>(node->next);
    if(node->type != Types::composite_t){
      Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->id.c_str());
    }else{
      auto symbol = getSymbol(tmp1->id);
      if(symbol != nullptr){
        symbol = symbol->typeTable!=nullptr?
          symbol->typeTable->getSymbol(tmp2->id):nullptr;
        if(symbol == nullptr)
          Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->id.c_str());
      }else{
        Errors::print(Errors::type_wrong_comp, tmp1->getTypeTxt(true), tmp2->id.c_str());
      }
    }
    auto symbol = getSymbol(tmp1->id);
    if(symbol && symbol->typeTable!=nullptr){
      node->next = symbol->typeTable->useVariable(node->next, useOfFunc);
      dynamic_cast<AST::Variable*>(node->next)->use = AST::read_comp;
    }
  }

  return node;
}

/* 'useOfFunc' significa que a variavel esta sendo usada
     no uso de uma função. Ex: c := soma(a, b);
    Isto é uma gambiarra para que possa ser usado arranjos
     como parametros de funções                             */
AST::Node* SymbolTable::_useVariable(AST::Node *node, bool useOfFunc){
  auto var = dynamic_cast<AST::Variable*>(node);
  var->use = AST::read;

  if ( ! checkId(var->id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], var->id.c_str());

  auto symbol = getSymbol(var->id);
  if ( symbol != nullptr ){
    var->type = symbol->type;
    var->compType = symbol->compTypeId;
    if(symbol->kind == Kinds::type_t ||
        ((!useOfFunc||var->next!=nullptr) && symbol->kind != Kinds::variable_t)){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        var->id.c_str(), Kinds::kindName[Kinds::variable_t]);
      var->type = Types::unknown_t;
      var->compType = "";
    }else if(symbol->type != Types::composite_t && !useOfFunc && checkId(var->id,true) && !symbol->initialized){
      Errors::print(Errors::not_initialized, Kinds::kindName[Kinds::variable_t],
        var->id.c_str());
    }
  }
  if(useOfFunc && symbol != nullptr && symbol->kind == Kinds::array_t  && var->next == nullptr){
    auto arr = new AST::Array(var->id, var->next, nullptr, AST::read,
        symbol->aSize, var->compType, var->type);
    delete node;
    return arr;
  }else
    return var;
}

AST::Node* SymbolTable::_useArray(AST::Node *node, bool useOfFunc){
  auto arr = dynamic_cast<AST::Array*>(node);
  arr->use = AST::read;

  if( !checkId(arr->id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::array_t], arr->id.c_str());

  auto symbol = getSymbol(arr->id);
  if(symbol != nullptr){
    if(symbol->kind != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        arr->id.c_str(), Kinds::kindName[Kinds::array_t]);
    else{
      arr->type = symbol->type;
      arr->compType = symbol->compTypeId;
      arr->size = symbol->aSize;
    }
  }
  if(arr->index == nullptr || arr->index->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[arr->index->type]);

  return arr;
}

AST::Node* SymbolTable::useFunc(std::string id, AST::Node *params){
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
    if(symbol->kind != Kinds::function_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::function_t]);
    else
      type = symbol->type;

    auto dparams = symbol->params;
    while(uparams != nullptr){
      if(dparams != nullptr){
        // param de tipo imcompativel [15]
        if(uparams->type != dparams->type)
          Errors::print(Errors::param_wrong_type,Types::mascType[dparams->type],
            Types::mascType[uparams->type]);
        if(dparams->getNodeType()  == AST::array_nt){
          auto uparams2 = uparams;
          // Gambiarra para resolver coisas como: arranjador((v));
          if(uparams->getNodeType() == AST::uniop_nt){
            auto tmp = dynamic_cast<AST::UniOp*>(uparams);
            while(tmp!=nullptr && tmp->op == Ops::u_paren){
              if(tmp->right->getNodeType() == AST::uniop_nt)
                tmp = dynamic_cast<AST::UniOp*>(tmp->right);
              else{
                uparams2 = tmp->right;
                tmp = nullptr;
              }
            }
          }
          if(uparams2->getNodeType() != AST::array_nt)
            Errors::print(Errors::param_value_as_array);
          else{
            // tamanho do arranjo tem q ser maior ou igual ao do param [16]
            auto ar = dynamic_cast<AST::Array*>(uparams2);
            if(ar->size < dynamic_cast<AST::Array*>(dparams)->size)
              Errors::print(Errors::array_size_lst_needed, ar->id.c_str());
          }
        }
        ++nd; dparams = dparams->next;
      }
      ++nu; uparams = uparams->next;
    }
    while(dparams != nullptr){
      ++nd; dparams = dparams->next;
    }
  }else{
    while(uparams != nullptr){
      ++nu; uparams = uparams->next;
    }
    defined = false;
  }

  // numero de parametros [23]
  if(nd != nu)
    Errors::print(Errors::func_wrong_param_amount,
      id.c_str(), nd, nu);

  // função não declarada deve retornar nó com 0 parametros [25]
  return new AST::Function(id, (defined?params:nullptr), nullptr, AST::read, type);
}

/* Seta as variaveis do Symbol referentes
 *  ao tipo
 */
void Symbol::setType(Types::Type t, std::string tId, SymbolTable* tTable){
  type = t;
  if(t == Types::composite_t){
    compTypeId = tId;
    typeTable = tTable;
  }
}

/* Seta o tipo de toda a sequencia de variaveis
 */
void SymbolTable::setType(AST::Node *node, Types::Type type, std::string compType){
  AST::Variable *var = dynamic_cast<AST::Variable*>(node);
  Symbol *varSymbol = nullptr;
  Symbol *ctSymbol = getSymbol(compType);

  if(type==Types::composite_t && ctSymbol == nullptr)
    Errors::print(Errors::type_undefined, compType.c_str());
  else{
    while(var != nullptr){
      varSymbol = getSymbol(var->id);
      if(varSymbol->type == Types::unknown_t){
        // Cada variavel tem que ter uma cópia das variaveis
        //  do corpo do tipo [tabela de simbolos do tipo]
        varSymbol->setType(type, compType, ctSymbol==nullptr?
          nullptr:ctSymbol->typeTable->copy());
        var->setType(type, compType);
      }

      var = dynamic_cast<AST::Variable*>(var->next);
    }
  }
}

void SymbolTable::setArraySize(AST::Node *node, int aSize){
  AST::Array *tmp = (AST::Array*) node;
  while(tmp != nullptr){
    tmp->setSize(aSize);
    getSymbol(tmp->id)->aSize = aSize;
    tmp = (AST::Array*) tmp->next;
  }
}

/* Adiciona os parametros da declaração ou definição
 *  de uma função ao escopo do corpo dela
 */
void SymbolTable::addFuncParams(AST::Node *oldParams, AST::Node *newParams){
  AST::Variable *var = nullptr;
  if(oldParams == nullptr)// Função ja foi declarada
    var = (AST::Variable*) newParams;
  else
    var = (AST::Variable*) oldParams;

  while(var != nullptr){
    Symbol *entry = new Symbol(var->getKind());
    entry->type = var->type;
    entry->initialized = true;
    if(var->getNodeType() == AST::array_nt)
      entry->aSize = dynamic_cast<AST::Array*>(var)->size;

    addSymbol(var->id,entry);

    var = (AST::Variable*) var->next;
  }
}

/* Verifica se todas as funções declaradas
 *  foram definidas
*/
void SymbolTable::checkFuncs(){
  for(const auto& iter : _entryList){
    const auto& symbol = iter.second;
    if(symbol->kind == Kinds::function_t && !symbol->initialized)
      Errors::print(Errors::func_never_declared, iter.first.c_str());
  }
}

Symbol* Symbol::copy(){
  Symbol *s = new Symbol(this->kind);
  s->type = this->type;
  s->initialized = this->initialized;
  s->params = this->params;
  s->aSize = this->aSize;
  s->compTypeId = this->compTypeId;
  s->typeTable = this->typeTable!=nullptr?
    this->typeTable->copy():nullptr;

  return s;
}

SymbolTable* SymbolTable::copy(){
  SymbolTable *st = new SymbolTable(this->_previous);
  for(auto& iter : _entryList){
    auto& symbol = iter.second;
    st->addSymbol(iter.first, symbol->copy());
  }
  return st;
}
