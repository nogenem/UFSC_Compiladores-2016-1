#pragma once

#include <map>
#include "util.hpp"

namespace AST{ class Node; }// Odeio 'circular reference'...

namespace ST {

class SymbolTable;
class Symbol;

typedef std::map<std::string, Symbol*> SymbolList; //Set of Symbols

class Symbol {
  public:
    Symbol(Kinds::Kind k): kind(k) {}
    Symbol(AST::Node *params, Types::Type type): kind(Kinds::function_t), type(type),
      params(params) {}

    void setType(Types::Type t, std::string tId, SymbolTable* tTable);
    Symbol* copy();

    Kinds::Kind kind;
    Types::Type type=Types::unknown_t;
    bool initialized=false;// Initialized/Defined?
    AST::Node *params=nullptr;// Parametros de funções
    int aSize=0;// Array size

    std::string compTypeId;// Nome do tipo composto
    SymbolTable *typeTable=nullptr;// Tabela de simbolos do tipo composto
};

class SymbolTable {
  public:
    SymbolTable(SymbolTable *prev) : _previous(prev) {}
    ~SymbolTable() {}

    bool checkId(std::string id, bool creation=false);
    void addSymbol(std::string id, Symbol *newsymbol);

    AST::Node* newVariable(std::string id, AST::Node* next, bool isArray,
      bool insideType=false);
    AST::Node* declFunction(std::string id, AST::Node *params, Types::Type type);
    AST::Node* defFunction(std::string id, AST::Node *params, AST::Node *block,
      Types::Type type);
    AST::Node* defCompType(std::string id, AST::Node *block);
    void assignVariable(AST::Node *var);
    AST::Node* useVariable(AST::Node *node, bool useOfFunc);
    AST::Node* useFunc(std::string id, AST::Node *params);

    void setType(AST::Node *node, Types::Type type, std::string compType);
    void setArraySize(AST::Node *node, int aSize);
    void addFuncParams(AST::Node *oldParams, AST::Node *newParams);
    void checkFuncs();

    SymbolList& getEntryList(){return _entryList;}
    SymbolTable* getPrevious(){return _previous;}
    Symbol* getSymbol(std::string id);

    SymbolTable* copy();

  private:
    AST::Node* _useVariable(AST::Node *node, bool useOfFunc);
    AST::Node* _useArray(AST::Node *node, bool useOfFunc);

    // Lista de simbolos
    SymbolList _entryList;
    // 'Escopo' anterior
    SymbolTable *_previous;
};

}
