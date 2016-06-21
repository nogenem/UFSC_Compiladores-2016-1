/*
 * at.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <map>

#include "ast.hpp"

namespace AT {

typedef std::map<int,AST::Node*> ArrValues;

class Symbol {
public:
	// constructors
	Symbol(){ ++_refs; }

	// other funcs
	void addRef(){++_refs;}
	void subRef(){--_refs;}

	// getters
	AST::Node* getValue(int index);
	int geRefs(){return _refs;}
	// setters
	void setValue(int index, AST::Node* value);
private:
	ArrValues _values;
	int _refs;//numero de referencias a este array
};

typedef std::map<int, Symbol*> ArrAddrs;

class ArrayTable {
public:
	// constructors
	ArrayTable(){}

	// other funcs
	Symbol* createArray();

	// getters
	Symbol* getArray(int addr);
	// setters

private:
	ArrAddrs _addrs;
	static int _n;
};
}
