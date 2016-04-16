/*Symbol Table definition*/
#pragma once

#include <map>
#include "ast.h"
#include "Variant_t.h"

extern void yyerror(const char* s, ...);

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
        SymbolList entryList;
        SymbolTable() {}
        /*checkId returns true if the variable has been defined and false if it does not exist*/
        bool checkId(std::string id) {return entryList.find(id) != entryList.end();}
        void addSymbol(std::string id, Symbol newsymbol) {entryList[id] = newsymbol;}
        AST::Node* newVariable(std::string id, AST::Node* next);
        AST::Node* assignVariable(std::string id);
        AST::Node* useVariable(std::string id);
        AST::Node* setType(AST::Node *node, std::string type);
};

}
