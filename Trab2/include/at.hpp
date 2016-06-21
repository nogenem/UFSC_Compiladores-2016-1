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
	Symbol(){}

	// getters
	AST::Node* getValue(int index);
	// setters
	void setValue(int index, AST::Node* value);
private:
	ArrValues _values;
};

typedef std::map<int, Symbol*> ArrAddrs;

class ArrayTable {
public:
	// constructors
	ArrayTable(){}

	// other funcs

	// getters
	Symbol* getArray(int addr);
	// setters

private:
	ArrAddrs _addrs;
	static int _n;
};
}
