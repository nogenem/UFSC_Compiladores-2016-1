/*
 * st.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <map>
#include <string>

#include "util.hpp"

namespace AST {
class Node;
class Variable;
} /* namespace AST */

namespace ST{

class SymbolTable;
class Symbol;

typedef std::map<std::string, Symbol*> SymbolList; //Set of Symbols

class Symbol {
public:
	// constructors
	Symbol(Types::Type type):
		_type(type), _value(0){}
	// destructors
	~Symbol(){}

	// getters
	const Types::Type getType(){return _type;}
	const int getValue(){return _value;}
	// setters
	void setType(Types::Type type){_type=type;}
	void setValue(int value){_value=value;}
private:
	Types::Type _type;
	// value booleano, inteiro ou 'endereço'
	//  de um arranjo e uma função
	int _value;
};

class SymbolTable {
public:
	// constructors
	SymbolTable(SymbolTable* prev):
		_previous(prev){}
	// destructors
	~SymbolTable(){}

	// other funcs
	bool checkId(std::string id, bool creation=false);
	void addSymbol(std::string id, Symbol *newsymbol);

	AST::Node* declVar(AST::Node *varlist, AST::Node *exprlist);
	AST::Node* assignVar(AST::Node *varlist, AST::Node *exprlist);
	AST::Node* useVar(std::string id, AST::Node *index);

	// getters
	SymbolTable* getPrevious(){return _previous;}
	Symbol* getSymbol(std::string id);
	// setters
	void setPrevious(SymbolTable *prev){_previous=prev;}
private:
	// other funcs
	AST::Node* _newVar(AST::Node *varlist);
	AST::Node* _assignVar(AST::Variable *var, AST::Node *expr);
private:
	SymbolList _entryList;
	SymbolTable *_previous;
};

}
