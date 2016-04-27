#pragma once

#include <map>

extern void yyerror(const char* s, ...);

namespace AST{ class Node; }// Odeio 'circular reference'...

namespace ST {

class Symbol;

enum Kind { variable_t };
enum Type { integer_t, real_t, bool_t };
typedef std::map<std::string, Symbol*> SymbolList; //Set of Symbols

class Symbol {
  public:
    Symbol(Kind k): kind(k), type(integer_t), initialized(false) {}

    Kind kind;
    Type type;
    bool initialized;//initialized/defined?
};

class SymbolTable{
  public:
    SymbolTable(SymbolTable *prev) : _previous(prev) {}
    ~SymbolTable() {}

    bool checkId(std::string id, bool creation=false);
    void addSymbol(std::string id, Symbol *newsymbol);
    AST::Node* newVariable(std::string id, AST::Node* next, bool declaration=false);
    AST::Node* assignVariable(std::string id);
    AST::Node* useVariable(std::string id);
    AST::Node* setType(AST::Node *node, Type type);

    SymbolList& getEntryList(){return _entryList;}
    SymbolTable* getPrevious(){return _previous;}
    Symbol* getSymbol(std::string id);

  private:
    SymbolList _entryList;
    SymbolTable *_previous;
};

}
