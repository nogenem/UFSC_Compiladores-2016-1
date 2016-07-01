/*
 * ft.cpp
 *
 *  Created on: 21 de jun de 2016
 *      Author: Gilne
 */

#include "../include/ft.hpp"

using namespace FT;

// Symbol

// FuncTable

int FuncTable::_n = 0;

// other funcs

/**
 * Função responsavel por criar uma nova função
 *  e retornar o simbolo desta nova função
 */
Symbol* FuncTable::createFunc(AST::Function *func){
	int n = _n++;
	auto symbol = new Symbol(n, func);
	_addrs[n] = symbol;
	return symbol;
}

/**
 * Função que adiciona +1 para o valor de referencias
 *  a função com endereço addr
 *
 * @param addr	Endereço da função
 */
void FuncTable::plusRef(int addr){
	auto symbol = getFunc(addr);
	if(symbol != nullptr)
		symbol->_plusRef();
}

/**
 * Função que subtrai 1 do valor de referencias
 *  a função com endereço addr
 * Caso o numero de referencias chegue a zero,
 *  o simbolo da função é deletado e o endereço
 *  desta função é removido.
 *
 * @param addr	Endereço da função
 */
void FuncTable::minusRef(int addr){
	auto symbol = getFunc(addr);
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
 * Função responsavel por retornar o simbolo da função
 *  com endereço addr
 *
 * @param addr	Endereço da função que se quer o simbolo
 */
Symbol* FuncTable::getFunc(int addr){
	if(_addrs.find(addr) != _addrs.end())
		return _addrs[addr];
	else
		return nullptr;
}
