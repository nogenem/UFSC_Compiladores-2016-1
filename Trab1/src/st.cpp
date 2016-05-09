#include "st.hpp"
#include "ast.hpp"

using namespace ST;

extern void yyerror(const char* s, ...);

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
  if ( checkId(id, true) ) yyerror("semantico: redefinicao da variavel %s.", id.c_str());
  else {
     Symbol *entry = new Symbol(isArray ? array_t : variable_t);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  if(isArray)
    return new AST::Array(id, next, AST::declr);
  else
    return new AST::Variable(id, next, AST::declr);
}

AST::Node* SymbolTable::newParam(std::string id, AST::Node *next, Types::Type type, bool isArray){
  if ( checkId(id, true) ) yyerror("semantico: redeclaracao do parametro %s.", id.c_str());
  else{
    Symbol *entry = new Symbol(isArray ? array_t : variable_t);
    entry->type = type;
    entry->initialized = true;
    addSymbol(id,entry);
  }
  return isArray ? new AST::Array(id, next, AST::param) :
    new AST::Variable(id, next, AST::param);
}

AST::Node* SymbolTable::declFunction(std::string id, AST::Node *params, Types::Type type){
  if ( checkId(id, true) ) yyerror("semantico: redeclaracao da funcao %s.", id.c_str());
  else {
    Symbol *entry = new Symbol(params, type);
    addSymbol(id, entry);
  }
  return new AST::Function(id, params, nullptr, AST::declr, type);
}

AST::Node* SymbolTable::assignVariable(std::string id){
  if ( ! checkId(id) ) yyerror("semantico: variavel %s nao declarada.", id.c_str());
  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != variable_t)
      yyerror("semantico: atribuicao esperava variavel e recebeu array.");
    symbol->initialized = true;
    type = symbol->type;
  }
  return new AST::Variable(id, NULL, AST::attr, type);
}

AST::Node* SymbolTable::assignArray(std::string id, AST::Node *index){
  if( !checkId(id) ) yyerror("semantico: variavel %s nao declarada.", id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != array_t)
      yyerror("semantico: atribuicao esperava array e recebeu variavel.");
    type = symbol->type;
  }
  if(index == nullptr || index->type != Types::integer_t)
    yyerror("semantico: indice espera um inteiro.");

  return new AST::Array(id, nullptr, index, AST::attr, type);
}

AST::Node* SymbolTable::useVariable(std::string id){
  if ( ! checkId(id) ) yyerror("semantico: variavel %s nao declarada.", id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if ( symbol != nullptr ){
    if(symbol->kind != variable_t)
      yyerror("semantico: esperava o uso de uma variavel mas recebeu um arranjo.");
    if(symbol->kind==variable_t && !symbol->initialized)
      yyerror("semantico: variavel %s nao inicializada.", id.c_str());
    type = symbol->type;
  }else{
    yyerror("semantico: variavel %s nao inicializada.", id.c_str());
  }

  return new AST::Variable(id, NULL, AST::read, type);
}

AST::Node* SymbolTable::useArray(std::string id, AST::Node *index){
  if( !checkId(id) ) yyerror("semantico: variavel %s nao declarada.", id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != array_t)
      yyerror("semantico: esperava o uso de um arranjo mas recebeu uma variavel.");
    type = symbol->type;
  }
  if(index == nullptr || index->type != Types::integer_t)
    yyerror("semantico: indice espera um inteiro.");

  return new AST::Array(id, nullptr, index, AST::read, type);
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
