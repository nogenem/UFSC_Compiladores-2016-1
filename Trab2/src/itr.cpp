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
	switch (type) {
		case Types::int_t:
			std::cout << ">> " << value << "\n";
			break;
		case Types::bool_t:
			std::cout << ">> " << (value==1?"true":"false") << "\n";
			break;
		case Types::unknown_t:
			std::cout << ">> " << "nil\n";
			break;
		default:
			break;
	}
}
