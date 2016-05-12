#include "st.hpp"
#include "ast.hpp"

using namespace ST;

bool SymbolTable::checkId(std::string id, bool creation/*=false*/){
  bool result = _entryList.find(id) != _entryList.end();
  if(creation)
    return result;
  return result ? result :
    (_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

void SymbolTable::addSymbol(std::string id, Symbol *newsymbol){
  _entryList[id] = newsymbol;
}

Symbol* SymbolTable::getSymbol(std::string id){
  bool result = _entryList.find(id) != _entryList.end();
  if(result)
    return _entryList[id];
  else if(_previous != nullptr)
    return _previous->getSymbol(id);
  else
    return nullptr;
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next, bool isArray){
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
    return new AST::Array(id, next, AST::declr);
  else
    return new AST::Variable(id, next, AST::declr);
}

AST::Node* SymbolTable::declFunction(std::string id, AST::Node *params, Types::Type type){
  if ( checkId(id, true) ){
    auto symbol = getSymbol(id);
    Errors::print(Errors::redefinition,
      Kinds::kindName[symbol->kind], id.c_str());
  }else {
    Symbol *entry = new Symbol(params, type);
    addSymbol(id, entry);
  }
  return new AST::Function(id, params, nullptr, AST::declr, type);
}

AST::Node* SymbolTable::assignVariable(std::string id){
  if ( ! checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], id.c_str());
  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::variable_t){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind], id.c_str(),
        Kinds::kindName[Kinds::variable_t]);
    }else{
      symbol->initialized = true;
      type = symbol->type;
    }
  }
  return new AST::Variable(id, NULL, AST::attr, type);
}

AST::Node* SymbolTable::assignArray(std::string id, AST::Node *index){
  if( !checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::function_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::array_t]);
    else
      type = symbol->type;
  }
  if(index == nullptr || index->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[index->type]);

  return new AST::Array(id, index, AST::attr, type);
}

AST::Node* SymbolTable::useVariable(std::string id){
  if ( ! checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if ( symbol != nullptr ){
    if(symbol->kind != Kinds::variable_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::variable_t]);
    if(symbol->kind==Kinds::variable_t && !symbol->initialized)
      Errors::print(Errors::not_initialized, Kinds::kindName[Kinds::variable_t],
        id.c_str());
    type = symbol->type;
  }else{
    Errors::print(Errors::not_initialized, Kinds::kindName[Kinds::variable_t],
      id.c_str());
  }

  return new AST::Variable(id, NULL, AST::read, type);
}

AST::Node* SymbolTable::useArray(std::string id, AST::Node *index){
  if( !checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::array_t], id.c_str());

  auto symbol = getSymbol(id);
  Types::Type type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::array_t]);
    type = symbol->type;
  }
  if(index == nullptr || index->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[index->type]);

  return new AST::Array(id, index, AST::read, type);
}

void SymbolTable::setType(AST::Node *node, Types::Type type){
  AST::Variable *tmp = (AST::Variable*) node;
  while(tmp != nullptr){
    tmp->setType(type);
    getSymbol(tmp->id)->type = type;
    tmp = (AST::Variable*) tmp->next;
  }
}

void SymbolTable::setArraySize(AST::Node *node, int aSize){
  AST::Array *tmp = (AST::Array*) node;
  while(tmp != nullptr){
    tmp->setSize(aSize);
    tmp = (AST::Array*) tmp->next;
  }
}
