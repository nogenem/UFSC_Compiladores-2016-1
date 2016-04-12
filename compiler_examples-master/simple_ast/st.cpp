#include "st.h"

using namespace ST;

extern SymbolTable symtab;

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next){
    if ( checkId(id) ) yyerror("Variable redefinition! %s\n", id.c_str());
    else {
       Symbol entry(Integer, variable, 0, false);
       addSymbol(id,entry); //Adds variable to symbol table
    }
    return new AST::Variable(id, next); //Creates variable node anyway
}

AST::Node* SymbolTable::setType(AST::Node *node, std::string type){
  Type t;
  if(type == "int")
    t = Integer;
  else if(type == "double")
    t = Double;
  else if(type == "long")
    t = Long;
  else if(type == "float")
    t = Float;

  AST::Variable *tmp = (AST::Variable*) node;
  while(tmp != NULL){
      entryList[tmp->id].type = t;
      tmp = (AST::Variable*) tmp->next;
  }
  return node;
}

AST::Node* SymbolTable::assignVariable(std::string id){
    if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
    entryList[id].initialized = true;
    return new AST::Variable(id, NULL); //Creates variable node anyway
}

AST::Node* SymbolTable::useVariable(std::string id){
    if ( ! checkId(id) ) yyerror("Variable not defined yet! %s\n", id.c_str());
    if ( ! entryList[id].initialized ) yyerror("Variable not initialized yet! %s\n", id.c_str());
    return new AST::Variable(id, NULL); //Creates variable node anyway
}
