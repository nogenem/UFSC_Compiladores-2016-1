/*
 * calctree.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include "../include/ast.hpp"
#include "../include/at.hpp"
#include "../include/st.hpp"
#include "../include/util.hpp"

using namespace AST;

extern AT::ArrayTable arrtab;

/**
 * Todas as funções desse arquivo são responsaveis por
 *  calcular o valor da sua subarvore, o jeito de calcular
 *  este valor varia de classe para classe.
 */

int Block::calcTree(ST::SymbolTable *scope){
	int value = 0;
	for(auto& line : _lines){
		value = line->calcTree(getScope());
		if(line->getNodeType() == AST::return_nt || line->isReturning()){
			setType(line->getType());
			setReturning(true);
			getScope()->removeRefs();
			return value;//retorna o valor do 1* 'return' encontrado
		}
	}
	getScope()->removeRefs();
	return 0;
}

int Variable::calcTree(ST::SymbolTable *scope){
	auto symbol = scope->getSymbol(getId());

	if(symbol==nullptr)
			Errors::throwErr(Errors::without_declaration, getId());

	auto stype  = symbol->getType();
	auto svalue = symbol->getValue();
	auto index = getIndex();

	if(index != nullptr){
		int iv = index->calcTree(scope);

		if(index->getType() != Types::int_t)
			Errors::throwErr(Errors::index_wrong_type, Types::mascType[index->getType()]);
		if(stype != Types::arr_t)
			Errors::throwErr(Errors::attempt_index, getId(), Types::mascType[stype]);

		auto arr = arrtab.getArray(symbol->getValue());
		auto val = arr->getValue(iv);
		int v = 0;

		if(val != nullptr){
			v = val->calcTree(scope);
			setType(val->getType());
		}else
			setType(Types::unknown_t);

		return v;
	}else{
		setType(stype);
		return svalue;
	}
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
	// Cria a nova array
	AT::Symbol* symbol = arrtab.createArray();
	auto tmp = _values;
	int index = 1;//index começa em 1
	// Adiciona todos os valores no simbolo da array
	while(tmp != nullptr){
		// Executa a expressão só para fazer as verificações
		tmp->calcTree(scope);
		if(tmp->getType() == Types::arr_t || tmp->getType() == Types::func_t){
			arrtab.minusRef(symbol->getAddr());//deleta a array
			Errors::throwErr(Errors::arr_type_not_allowed);
		}

		symbol->setValue(index, tmp);

		++index;
		tmp = tmp->getNext();
	}

	// Retorna o endereço da nova array criada
	return symbol->getAddr();
}

int Return::calcTree(ST::SymbolTable *scope){
	auto expr = getExpr();

	if(expr == nullptr) return 0;

	int value = expr->calcTree(scope);
	setType(expr->getType());
	return value;
}

int BinOp::_calcAssignArr(ST::SymbolTable *scope, Types::Type rtype, int rv){
	auto var = Variable::cast(getLeft());
	auto index = var->getIndex();
	int iv = index->calcTree(scope);

	if(rtype == Types::arr_t)
		Errors::throwErr(Errors::arr_type_not_allowed);

	auto symbol = scope->getSymbol(var->getId());
	auto arr = arrtab.getArray(symbol->getValue());

	arr->setValue(iv, getRight());

	return rv;
}

int BinOp::calcTree(ST::SymbolTable *scope){
	auto left = getLeft();
	auto right = getRight();

	int lv = left->calcTree(scope);
	int rv = right->calcTree(scope);

	setType(Types::binType(left->getType(), getOp(), right->getType()));

	if(getOp() == Ops::assign){
		auto var = Variable::cast(left);
		if(var->getIndex() != nullptr)
			return _calcAssignArr(scope, right->getType(), rv);

		auto symbol = scope->getSymbol(var->getId());
		symbol->setValue(rv, right->getType());
		return rv;
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
				Errors::throwErr(Errors::div_zero);
				return 0;//só pro eclipse não mostrar alerta
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
			return 0;
	}
}

int UniOp::calcTree(ST::SymbolTable *scope){
	auto right = getRight();

	int rv = right->calcTree(scope);

	setType(Types::unType(getOp(), right->getType()));

	switch (getOp()) {
		case Ops::u_paren:
			return rv;
		case Ops::u_not:
			return not rv;
		case Ops::u_minus:
			return -rv;
		default:
			return 0;
	}
}

int CondExpr::calcTree(ST::SymbolTable *scope){
	auto cond = getCond();
	int cv = cond->calcTree(scope);

	if(cond->getType() != Types::bool_t)
		Errors::throwErr(Errors::op_wrong_type, "teste", Types::mascType[Types::bool_t],
				Types::mascType[cond->getType()]);

	Block *branch = nullptr;

	if(cv==0) branch = getElseBranch();
	else branch = getThenBranch();

	if(branch == nullptr) return 0;

	int bv = branch->calcTree(scope);

	setType(branch->getType());
	setReturning(branch->isReturning());
	return bv;
}

int WhileExpr::calcTree(ST::SymbolTable *scope){
	auto cond = getCond();
	int cv = 0;
	auto block = getBlock();
	int bv = 0;

	while(true){
		cv = cond->calcTree(scope);

		if(cond->getType() != Types::bool_t)
			Errors::throwErr(Errors::op_wrong_type, "enquanto", Types::mascType[Types::bool_t],
					Types::mascType[cond->getType()]);

		if(cv == 0)
			return 0;

		bv = block->calcTree(scope);
		if(block->isReturning()){//teve um return
			setType(block->getType());
			setReturning(true);
			return bv;
		}
	}
}
