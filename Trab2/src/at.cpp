/*
 * at.cpp
 *
 *  Created on: 21 de jun de 2016
 *      Author: Gilne
 */

#include "../include/at.hpp"

using namespace AT;

// Symbol
AST::Node* Symbol::getValue(int index){
	if(_values.find(index) != _values.end())
		return _values[index];
	else
		return nullptr;
}

void Symbol::setValue(int index, AST::Node *value){
	if(_values.find(index) != _values.end()){
		delete _values[index];
	}
	_values[index] = value;
}

// Array Table
int ArrayTable::_n = 0;

Symbol* ArrayTable::getArray(int addr){
	if(_addrs.find(addr) != _addrs.end())
		return _addrs[addr];
	else
		return nullptr;
}
