/*Symbol Table definition*/
#pragma once

#include <map>
#include "Variant_t.h"

extern void yyerror(const char* s, ...);

namespace AST{ class Node; }// Odeio 'circular reference'...

namespace ST {

class Symbol;

enum Kind { variable };

typedef std::map<std::string,Symbol> SymbolList; //Set of Symbols

class Symbol {
    public:
        Kind kind;              /*Kind of symbol: variable, function, etc.*/
        bool initialized;       /*Defines if symbol has been initialized or not.*/
        Symbol() : kind(variable), _value(), initialized(false) {}
        Symbol(Kind k) : kind(k), _value(), initialized(false) {}

        void setValue(VAR::Variant_t v);
        VAR::Variant_t& getValue(){return _value;}
    private:
      VAR::Variant_t _value;   /*Space to store a value while we are doing interpretation.*/
};

class SymbolTable {
    public:
        SymbolTable() : _previous(nullptr) {}
        SymbolTable(SymbolTable *prev) : _previous(prev) {}
        ~SymbolTable() {}
        /*checkId returns true if the variable has been defined and false if it does not exist*/
        bool checkId(std::string id, bool creation=false);
        void addSymbol(std::string id, Symbol newsymbol);
        AST::Node* newVariable(std::string id, AST::Node* next);
        AST::Node* assignVariable(std::string id);
        AST::Node* useVariable(std::string id);
        AST::Node* setType(AST::Node *node, std::string type);

        SymbolList& getEntryList(){return _entryList;}
        SymbolTable* getPrevious(){return _previous;}
        Symbol& getSymbol(std::string id);
    private:
      SymbolList _entryList;
      SymbolTable *_previous;
};

}
