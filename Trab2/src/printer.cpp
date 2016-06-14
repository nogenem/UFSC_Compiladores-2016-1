/*
 * printer.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "ast.hpp"

using namespace AST;

void Block::printTree(){
	for(auto& line : _lines) {
		line->printTree();
		std::cout << std::endl;
	}
}

void Variable::printTree(){
	switch (_use) {
		case decl_u:
			std::cout << "Declaracao de variavel: " << getId();
			break;
		case attr_u:
			std::cout << "Atribuicao de valor para variavel " << getId();
			break;
		default:
			break;
	}
	auto next = getNext();
	if(next != nullptr){
		std::cout << "\n";
		next->printTree();
	}
}

void Value::printTree(){
	std::cout << "valor " << getTypeTxt(true) << " " << getN();
}

void Function::printTree(){

}

void Array::printTree(){

}

void Return::printTree(){
	std::cout << "Retorno de funcao: ";
	_expr->printTree();
}

void BinOp::printTree(){
	switch (_op) {
		case Ops::assign:
			getLeft()->printTree();
			std::cout << ": ";
			getRight()->printTree();
			break;
		default:
			std::cout << "(";
			_left->printTree();

			std::cout << " (" << Ops::opName[_op] << " ";
			if(Ops::masculineOp[_op]) std::cout << Types::mascType[getType()];
			else std::cout << Types::femType[getType()];
			std::cout << ") ";

			_right->printTree();
			std::cout << ")";
			break;
	}
	auto next = getNext();
	if(next != nullptr){
		//assign="\n"
		std::cout << "\n";
		next->printTree();
	}
}

void UniOp::printTree(){
	std::cout << "(";
	switch (_op) {
		case Ops::u_paren:
			std::cout << "(abre parenteses) ";
			_right->printTree();
			std::cout << " (fecha parenteses)";
			break;
		default:
			std::cout << "(" << Ops::opName[_op] << " ";
			if(Ops::masculineOp[_op]) std::cout << Types::mascType[getType()];
			else std::cout << Types::femType[getType()];
			std::cout << ") ";
			_right->printTree();
			break;
	}
	std::cout << ")";
}
