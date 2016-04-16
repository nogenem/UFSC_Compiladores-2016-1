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
        VAR::Variant_t value;   /*Space to store a value while we are doing interpretation.*/
        bool initialized;       /*Defines if symbol has been initialized or not.*/
        Symbol() : kind(variable), value(), initialized(false) {}
        Symbol(Kind k) : kind(k), value(), initialized(false) {}
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
