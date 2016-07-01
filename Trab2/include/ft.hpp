/*
 * ft.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <map>

#include "ast.hpp"

namespace FT {

typedef std::map<int,AST::Node*> FuncValues;

class Symbol {
public:
	// constructors
	Symbol(int addr, AST::Function* func):
		_func(func), _refs(0), _addr(addr){ }

	// destructors
	~Symbol(){}

	// getters
	AST::Function* getFunc(){return _func;}
	int geRefs(){return _refs;}
	int getAddr(){return _addr;}
	// setters

private:
	friend class FuncTable;
	// other funcs
	void _plusRef(){++_refs;}
	int _minusRef(){return --_refs;}
private:
	AST::Function* _func;//Nodo da função
	int _refs;//Numero de referencias a este array
	int _addr;//'Endereço' deste array
};

typedef std::map<int, Symbol*> FuncAddrs;

class FuncTable {
public:
	// constructors
	FuncTable(){}

	// other funcs
	Symbol* createFunc(AST::Function *func);
	void plusRef(int addr);
	void minusRef(int addr);

	// getters
	Symbol* getFunc(int addr);
private:
	// Tabela de funções
	//	[endereço] = simbolo da func
	FuncAddrs _addrs;
	// Numero que controla os endereços das
	//	funções
	static int _n;
};

}
