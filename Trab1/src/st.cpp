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

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next, bool declaration/*= false*/){
  if ( checkId(id, true) ) yyerror("Variable redefinition! %s\n", id.c_str());
  else {
     Symbol *entry = new Symbol(variable_t);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  return new AST::Variable(id, next, declaration); //Creates variable node anyway
}

AST::Node* SymbolTable::assignVariable(std::string id){
  if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
  auto symbol = getSymbol(id);
  if(symbol != nullptr)
    symbol->initialized = true;
  return new AST::Variable(id, NULL); //Creates variable node anyway
}

AST::Node* SymbolTable::useVariable(std::string id){
  if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
  auto symbol = getSymbol(id);
  if ( symbol != nullptr && !symbol->initialized ) yyerror("Variable not initialized yet! %s\n", id.c_str());
  return new AST::Variable(id, NULL); //Creates variable node anyway
}

AST::Node* SymbolTable::setType(AST::Node *node, Type type){
  AST::Variable *tmp = (AST::Variable*) node;
  while(tmp != nullptr){
    getSymbol(tmp->id)->type = type;
    tmp = (AST::Variable*) tmp->next;
  }
  return node;
}
