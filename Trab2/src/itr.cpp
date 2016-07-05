/*
 * itr.cpp
 *
 *  Created on: 17 de jun de 2016
 *      Author: Gilne
 */

#include <ast.hpp>
#include <itr.hpp>
#include <st.hpp>
#include <iostream>

using namespace ITR;

/**
 * Função responsavel por executar e, caso especificado,
 *  printar o resultado da expressão passada
 *
 * @param node	Nodo da expressão que se quer executar
 * @param print	TRUE caso queria printar o resultado da expressão
 * 				FALSE caso contrario
 */
void ITR::execExpr(AST::Node *node, bool print/*=false*/){
	// Só pode executar caso esteja no escopo
	//  global
	if(!symtab->isGlobal())
		return;

	AST::Node *tmp = node;
	int v = 0;
	while(tmp != nullptr){
		try{
			v = tmp->calcTree(symtab);
		}catch(int e){}
		if(print)
			_print(v, tmp->getType());

		tmp = tmp->getNext();
	}
}

/**
 * Função responsavel por printar um valor resultante
 *  de uma expressão.
 *
 * @param value	Valor a ser printado
 * @param type	Tipo do valor a ser printado
 */
void ITR::_print(int value, Types::Type type){
	std::cout << ">> ";
	switch (type) {
		case Types::int_t:
			std::cout << value;
			break;
		case Types::bool_t:
			std::cout << (value==0?"false":"true");
			break;
		case Types::unknown_t:
			std::cout << "nil";
			break;
		case Types::arr_t:
			std::cout << "array";
			break;
		case Types::func_t:
			std::cout << "function";
			break;
		default:
			break;
	}
	std::cout << std::endl;
}
