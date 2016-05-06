#pragma once

#include <map>

extern void yyerror(const char* s, ...);

namespace AST{ class Node; }// Odeio 'circular reference'...

namespace ST {

class Symbol;

enum Kind { variable_t, array_t };
enum Type { notype_t, integer_t, real_t, bool_t };
typedef std::map<std::string, Symbol*> SymbolList; //Set of Symbols

class Symbol {
  public:
    Symbol(Kind k): kind(k), type(integer_t),
      initialized(false), aSize(0) {}

    Kind kind;
    Type type;
    int aSize;//size of array
    bool initialized;//initialized/defined?
};

class SymbolTable {
  public:
    SymbolTable(SymbolTable *prev) : _previous(prev) {}
    ~SymbolTable() {}

    bool checkId(std::string id, bool creation=false);
    void addSymbol(std::string id, Symbol *newsymbol);
    AST::Node* newVariable(std::string id, AST::Node* next, bool isArray);
    AST::Node* assignVariable(std::string id);
    AST::Node* assignArray(std::string id, AST::Node *index);
    AST::Node* useVariable(std::string id);
    AST::Node* useArray(std::string id, AST::Node *index);
    void setType(AST::Node *node, Type type);
    void setArraySize(AST::Node *node, int aSize);

    SymbolList& getEntryList(){return _entryList;}
    SymbolTable* getPrevious(){return _previous;}
    Symbol* getSymbol(std::string id);

  private:
    SymbolList _entryList;
    SymbolTable *_previous;
};

}
