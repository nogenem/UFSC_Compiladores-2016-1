#pragma once

extern void yyerror(const char* s, ...);

namespace ST {

class Symbol;

enum Kind { variable_t };

class Symbol {
  public:
    Symbol(){}
};

class SymbolTable{
  public:
    SymbolTable(){}
};

}
