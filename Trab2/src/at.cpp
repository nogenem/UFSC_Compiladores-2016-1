/*
 * at.cpp
 *
 *  Created on: 21 de jun de 2016
 *      Author: Gilne
 */

#include "../include/at.hpp"


using namespace AT;

// Symbol

// getters
/**
 * Função que retorna o valor do indice do arranjo
 *
 * @param index	Indice que se quer o valor
 */
AST::Node* Symbol::getValue(int index){
	if(_values.find(index) != _values.end())
		return _values[index];
	else
		return nullptr;
}

// setters
/**
 * Função que altera o valor de um indice do arranjo
 *
 * @param index	Indice que se quer modificar o valor
 * @param value	Novo valor
 */
void Symbol::setValue(int index, AST::Node *value){
	_values[index] = value;
}

// Array Table

int ArrayTable::_n = 0;

// other funcs
/**
 * Função responsavel por criar um novo arranjo
 *  e retornar o simbolo deste novo arranjo
 */
Symbol* ArrayTable::createArray(){
	int n = _n++;
	auto symbol = new Symbol(n);
	_addrs[n] = symbol;
	return symbol;
}

/**
 * Função que adiciona +1 para o valor de referencias
 *  ao arranjo com endereço addr
 *
 * @param addr	Endereço do arranjo
 */
void ArrayTable::plusRef(int addr){
	auto symbol = getArray(addr);
	if(symbol != nullptr)
		symbol->_plusRef();
}

/**
 * Função que subtrai 1 do valor de referencias
 *  ao arranjo com endereço addr
 * Caso o numero de referencias chegue a zero,
 *  o simbolo do arranjo é deletado e o endereço
 *  deste arranjo é removido.
 *
 * @param addr	Endereço do arranjo
 */
void ArrayTable::minusRef(int addr){
	auto symbol = getArray(addr);
	if(symbol != nullptr){
		int r = symbol->_minusRef();
		if(r <= 0){
			delete symbol;
			_addrs.erase(addr);
		}
	}
}

// getters
/**
 * Função responsavel por retornar o simbolo do arranjo
 *  com endereço addr
 *
 * @param addr	Endereço do arranjo que se quer o simbolo
 */
Symbol* ArrayTable::getArray(int addr){
	if(_addrs.find(addr) != _addrs.end())
		return _addrs[addr];
	else
		return nullptr;
}
