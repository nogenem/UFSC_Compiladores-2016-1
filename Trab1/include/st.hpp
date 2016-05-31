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
  // constructors
  Symbol(Kinds::Kind kind):
    _kind(kind){}
  Symbol(AST::Node *params, Types::Type type):
    _params(params), _type(type), _kind(Kinds::function_t){}

  // destructors
  ~Symbol(){}

  // other funcs
  void setType(Types::Type type, std::string compType,
    SymbolTable* typeTable);
  Symbol* copy();
  void initializeAllComps();

  // getters
  Kinds::Kind getKind(){return _kind;}
  Types::Type getType(){return _type;}
  bool isInitialized(){return _initialized;}
  int getArraySize(){return _aSize;}
  AST::Node* getParams(){return _params;}
  std::string getCompType(){return _compType;}
  SymbolTable* getTypeTable(){return _typeTable;}
  // setters
  void setKind(Kinds::Kind kind){_kind=kind;}
  void setType(Types::Type type){_type=type;}
  void setInitialized(bool initialized){_initialized=initialized;}
  void setArraySize(int aSize){_aSize=aSize;}
  void setParams(AST::Node *params){_params=params;}
  void setCompType(std::string compType){_compType=compType;}
  void setTypeTable(SymbolTable *typeTable){_typeTable=typeTable;}
private:
  Kinds::Kind _kind=Kinds::unknown_t;
  Types::Type _type=Types::unknown_t;
  bool _initialized=false;// Inicializado/Definido?
  int _aSize=-1;// Tamanho do arranjo
  AST::Node *_params=nullptr;// Parametros da função

  std::string _compType="";// Tipo composto
  // Tabela de simbolos do tipo composto
  // Usado para saber quais componentes as variaveis podem ter
  //  e para pegar seus dados
  SymbolTable *_typeTable=nullptr;
};

class SymbolTable {
public:
  // constructors
  SymbolTable(SymbolTable *prev):
    _previous(prev){}

  // destructors
  ~SymbolTable(){}//TODO?

  // other funcs
  void assignVariable(AST::Node *node);
  AST::Node* declFunction(AST::Node *params, std::string id,
    Types::Type type);
  AST::Node* defFunction(AST::Node *params, AST::Node *block, std::string id,
    Types::Type type);
  AST::Node* defCompType(std::string id, AST::Node *block, SymbolTable *st);
  AST::Node* newVariable(std::string id, AST::Node *next, bool isArray,
    bool insideType=false);
  AST::Node* useVariable(AST::Node *node, bool useOfFunc);
  AST::Node* useFunc(AST::Node *params, std::string id);
  void addFuncParams(AST::Node *oldParams, AST::Node *newParams);
  void checkFuncs();
  void initializeAllVars();

  void setType(AST::Node *node, Types::Type type, std::string compType);
  void setArraySize(AST::Node *node, int size);

  bool checkId(std::string id, bool creation=false);
  void addSymbol(std::string id, Symbol *newsymbol);
  SymbolTable* copy();

  // getters
  SymbolList& getEntryList(){return _entryList;}
  SymbolTable* getPrevious(){return _previous;}
  Symbol* getSymbol(std::string id);
  // setters
private:
  // other funcs
  AST::Node* _useVariable(AST::Node *node, bool useOfFunc);
  AST::Node* _useArray(AST::Node *node, bool useOfFunc);
  void _checkIndex(AST::Node *node);
private:
  // Lista de simbolos
  SymbolList _entryList;
  // 'Escopo' anterior
  SymbolTable *_previous;
};

}
