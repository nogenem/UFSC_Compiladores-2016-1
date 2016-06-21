/*
 * calctree.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include <cstdlib>
#include <string>

#include "../include/ast.hpp"
#include "../include/at.hpp"
#include "../include/st.hpp"
#include "../include/util.hpp"

using namespace AST;

extern AT::ArrayTable arrtab;

int Block::calcTree(ST::SymbolTable *scope){
	int value = 0;
	for(auto& line : _lines){
		value = line->calcTree(_scope);
		if(line->getNodeType() == AST::return_nt)
			return value;//retorno o valor do 1* 'return' encontrado
	}
	return 0;
}

int Variable::calcTree(ST::SymbolTable *scope){
	if(getError())
		return 0;

	auto symbol = scope->getSymbol(getId());
	if(symbol != nullptr){
		setType(symbol->getType());
		return symbol->getValue();
	}else{
		Errors::print(Errors::without_declaration, getId());
		setError(true);
	}
	return 0;
}

int Value::calcTree(ST::SymbolTable *scope){
	int value = atoi(_n.c_str());
	if(_n=="true") value = 1;
	else if(_n=="false") value = 0;

	return value;
}

int Function::calcTree(ST::SymbolTable *scope){
	return 0;
}

int Array::calcTree(ST::SymbolTable *scope){
	AT::Symbol* symbol = arrtab.createArray();
	auto tmp = _values;
	int index = 1;//index começa em 1
	while(tmp != nullptr){
		// executa a expressão só para fazer as verificações
		tmp->calcTree(scope);
		if(!tmp->getError())// só adiciona se não tiver erros
			symbol->setValue(index, tmp);
		++index;

		tmp = tmp->getNext();
	}

	return 0;
}

int Return::calcTree(ST::SymbolTable *scope){
	int value = getExpr()->calcTree(scope);
	setType(getExpr()->getType());
	return value;
}

int BinOp::calcTree(ST::SymbolTable *scope){
	auto left = getLeft();
	auto right = getRight();

	int lv = left->calcTree(scope);
	int rv = right->calcTree(scope);

	setType(Types::binType(left->getType(), getOp(), right->getType()));

	if(getOp()==Ops::assign && !left->getError()){
		auto var = Variable::cast(left);
		auto symbol = scope->getSymbol(var->getId());
		symbol->setValue(rv);
		symbol->setType(right->getType());
		return rv;
	}

	if(getType() == Types::unknown_t || left->getError()){
		setError(true);
		return 0;
	}
	switch (getOp()) {
		case Ops::b_and:
			return lv && rv;
		case Ops::b_minus:
			return lv - rv;
		case Ops::b_or:
			return lv || rv;
		case Ops::division:{
			if(rv == 0){
				Errors::print(Errors::div_zero);
				break;
			}else
				return lv / rv;
		}case Ops::eq:
			return lv == rv;
		case Ops::grt:
			return lv > rv;
		case Ops::grteq:
			return lv >= rv;
		case Ops::lst:
			return lv < rv;
		case Ops::lsteq:
			return lv <= rv;
		case Ops::neq:
			return lv != rv;
		case Ops::plus:
			return lv + rv;
		case Ops::times:
			return lv * rv;
		default:
			break;
	}
	return 0;
}

int UniOp::calcTree(ST::SymbolTable *scope){
	auto right = getRight();

	int rv = right->calcTree(scope);

	setType(Types::unType(getOp(), right->getType()));

	if(getType() == Types::unknown_t){
		setError(true);
		return 0;
	}

	switch (getOp()) {
		case Ops::u_paren:
			return rv;
		case Ops::u_not:
			return not rv;
		case Ops::u_minus:
			return -rv;
		default:
			break;
	}
	return 0;
}
