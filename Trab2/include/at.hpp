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
	Symbol(int addr):
		_addr(addr), _refs(1){ }

	// getters
	AST::Node* getValue(int index);
	int geRefs(){return _refs;}
	int getAddr(){return _addr;}
	// setters
	void setValue(int index, AST::Node* value);
private:
	friend class ArrayTable;
	// other funcs
	void _plusRef(){++_refs;}
	int _minusRef(){return --_refs;}
private:
	// Tabela dos valores do arranjo
	//	[index] = valor
	ArrValues _values;
	int _refs;//Numero de referencias a este array
	int _addr;//'Endereço' deste array
};

typedef std::map<int, Symbol*> ArrAddrs;

class ArrayTable {
public:
	// constructors
	ArrayTable(){}

	// other funcs
	Symbol* createArray();
	void plusRef(int addr);
	void minusRef(int addr);

	// getters
	Symbol* getArray(int addr);
	// setters

private:
	// Tabela de arranjos
	//	[endereço] = simbolo do arranjo
	ArrAddrs _addrs;
	// Numero que controla os endereços dos
	//  arranjos
	static int _n;
};
}
