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

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next,
      bool isArray){
  if ( checkId(id, true) ) yyerror("semantico: redefinicao da variavel %s.\n", id.c_str());
  else {
     Symbol *entry = new Symbol(isArray ? array_t : variable_t);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  if(isArray)
    return new AST::Array(id, next, AST::declr);
  else
    return new AST::Variable(id, next, AST::declr);
}

AST::Node* SymbolTable::assignVariable(std::string id){
  if ( ! checkId(id) ) yyerror("semantico: variavel %s nao declarada.\n", id.c_str());
  auto symbol = getSymbol(id);
  if(symbol != nullptr){
    if(symbol->kind != variable_t)
      yyerror("semantico: atribuicao esperava variavel e recebeu array.\n");
    symbol->initialized = true;
  }
  return new AST::Variable(id, NULL, AST::attr); //Creates variable node anyway
}

AST::Node* SymbolTable::assignArray(std::string id, AST::Node *index){
  if( !checkId(id) ) yyerror("semantico: variavel %s nao declarada.\n", id.c_str());

  auto symbol = getSymbol(id);
  if(symbol != nullptr && symbol->kind != array_t)
    yyerror("semantico: atribuicao esperava array e recebeu variavel.\n");
  if(index == nullptr || index->getType() != integer_t)
    yyerror("semantico: indice espera um inteiro.\n");

  return new AST::Array(id, nullptr, index, AST::attr);
}

AST::Node* SymbolTable::useVariable(std::string id){
  if ( ! checkId(id) ) yyerror("semantico: variavel %s nao declarada.\n", id.c_str());

  auto symbol = getSymbol(id);
  if ( symbol != nullptr ){
    if(symbol->kind != variable_t)
      yyerror("semantico: esperava o uso de uma variavel e recebeu um array.\n");
    if(symbol->kind==variable_t && !symbol->initialized)
      yyerror("semantico: variavel %s nao inicializada.\n", id.c_str());
  }

  return new AST::Variable(id, NULL, AST::read); //Creates variable node anyway
}

AST::Node* SymbolTable::useArray(std::string id, AST::Node *index){
  if( !checkId(id) ) yyerror("semantico: variavel %s nao declarada.\n", id.c_str());

  auto symbol = getSymbol(id);
  if(symbol != nullptr && symbol->kind != array_t)
    yyerror("semantico: esperava o uso de um array e recebeu uma variavel.\n");
  if(index == nullptr || index->getType() != integer_t)
    yyerror("semantico: indice espera um inteiro.\n");

  return new AST::Array(id, nullptr, index, AST::read);
}

void SymbolTable::setType(AST::Node *node, Type type){
  AST::Variable *tmp = (AST::Variable*) node;
  while(tmp != nullptr){
    getSymbol(tmp->id)->type = type;
    tmp = (AST::Variable*) tmp->next;
  }
}

void SymbolTable::setArraySize(AST::Node *node, int aSize){
  AST::Array *tmp = (AST::Array*) node;
  while(tmp != nullptr){
    getSymbol(tmp->id)->aSize = aSize;
    tmp = (AST::Array*) tmp->next;
  }
}
