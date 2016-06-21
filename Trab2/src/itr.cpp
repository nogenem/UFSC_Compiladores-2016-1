/*
 * itr.cpp
 *
 *  Created on: 17 de jun de 2016
 *      Author: Gilne
 */

#include "../include/itr.hpp"

#include <iostream>

#include "../include/ast.hpp"
#include "../include/st.hpp"

using namespace ITR;

void ITR::execExpr(AST::Node *node, bool print/*=false*/){
	// Soh pode executar caso esteja no escopo
	//  global
	if(!symtab->isGlobal())
		return;

	AST::Node *tmp = node;
	int v = 0;
	while(tmp != nullptr){
		if(!tmp->getError()){
			v = tmp->calcTree(symtab);
			if(print && !tmp->getError())
				_print(v, tmp->getType());
		}
		tmp = tmp->getNext();
	}
}

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
		default:
			break;
	}
	std::cout << std::endl;
}
