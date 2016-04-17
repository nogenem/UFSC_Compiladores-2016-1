#include "st.h"
#include "ast.h"

using namespace ST;

extern SymbolTable *symtab;

void Symbol::setValue(VAR::Variant_t v){
  switch (v.getType()) {
    case VAR::integer_t: _value.setValue(v.getIntValue()); break;
    case VAR::double_t: _value.setValue(v.getDoubleValue()); break;
  }
}

bool SymbolTable::checkId(std::string id, bool creation){
  bool result = _entryList.find(id) != _entryList.end();
  if(creation)
    return result;
  return result ? result :
    (_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

void SymbolTable::addSymbol(std::string id, Symbol newsymbol) {
  _entryList[id] = newsymbol;
}

Symbol& SymbolTable::getSymbol(std::string id){
  bool result = _entryList.find(id) != _entryList.end();
  if(result)
    return _entryList[id];
  else if(_previous != nullptr)
    return _previous->getSymbol(id);
  else{
    yyerror("Variable not defined yet! %s\n", id.c_str());
    //return NULL;
  }
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next){
    if ( checkId(id, true) ) yyerror("Variable redefinition! %s\n", id.c_str());
    else {
       Symbol entry(variable);
       addSymbol(id,entry); //Adds variable to symbol table
    }
    return new AST::Variable(id, next, symtab); //Creates variable node anyway
}

AST::Node* SymbolTable::setType(AST::Node *node, std::string type){
  VAR::Type t;
  if(type == "int")
    t = VAR::integer_t;
  else if(type == "double")
    t = VAR::double_t;

  AST::Variable *tmp = (AST::Variable*) node;
  while(tmp != NULL){
      getSymbol(tmp->id).getValue().setType(t);
      tmp = (AST::Variable*) tmp->next;
  }
  return node;
}

AST::Node* SymbolTable::assignVariable(std::string id){
    if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
    getSymbol(id).initialized = true;
    return new AST::Variable(id, NULL, symtab); //Creates variable node anyway
}

AST::Node* SymbolTable::useVariable(std::string id){
    if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
    if ( ! getSymbol(id).initialized ) yyerror("Variable not initialized yet! %s\n", id.c_str());
    return new AST::Variable(id, NULL, symtab); //Creates variable node anyway
}
