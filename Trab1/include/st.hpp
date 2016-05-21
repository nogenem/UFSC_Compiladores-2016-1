#pragma once

#include <map>
#include "util.hpp"

namespace AST{ class Node; }// Odeio 'circular reference'...

namespace ST {

class Symbol;

typedef std::map<std::string, Symbol*> SymbolList; //Set of Symbols

class Symbol {
  public:
    Symbol(Kinds::Kind k): kind(k), type(Types::unknown_t),
      initialized(false), params(nullptr) {}
    Symbol(AST::Node *params, Types::Type type): kind(Kinds::function_t), type(type),
      initialized(false), params(params) {}

    Kinds::Kind kind;
    Types::Type type;
    bool initialized;// Initialized/Defined?
    AST::Node *params;// Parametros de funções
    int aSize;// Array size
    AST::Node *typeBlock;// Bloco de um tipo composto
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
    AST::Node* assignVariable(std::string id);
    AST::Node* assignArray(std::string id, AST::Node *index);
    AST::Node* useVariable(std::string id,bool useOfFunc);
    AST::Node* useArray(std::string id, AST::Node *index);
    AST::Node* useFunc(std::string id, AST::Node *params);

    void setType(AST::Node *node, Types::Type type);
    void setArraySize(AST::Node *node, int aSize);
    void addFuncParams(AST::Node *oldParams, AST::Node *newParams);
    void checkFuncs();

    SymbolList& getEntryList(){return _entryList;}
    SymbolTable* getPrevious(){return _previous;}
    Symbol* getSymbol(std::string id);

  private:
    // Lista de simbolos
    SymbolList _entryList;
    // 'Escopo' anterior
    SymbolTable *_previous;
};

}
