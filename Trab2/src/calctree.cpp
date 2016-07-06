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
#include "../include/ft.hpp"
#include "../include/st.hpp"
#include "../include/util.hpp"

using namespace AST;

extern AT::ArrayTable arrtab;
extern FT::FuncTable functab;


/**
 * Problema da cópia do bloco de função:
 * 	Eu sou obrigado a fazer uma cópia da ST do bloco de funções
 * 	 para que não ocorra problemas do tipo:
 * 		local fun = function()
 * 			local b = c;
 * 			local c = 5;
 * 		end;
 *
 * 	Só que fazendo isso, blocos internos,
 * 	 como os do IF, WHILE e outras funções, ficam com um
 * 	 ponteiro apontando para a versão original da ST
 * 	 gerando assim outros problemas.
 */


/**
 * Todas as funções desse arquivo são responsaveis por
 *  calcular o valor da sua subarvore, o jeito de calcular
 *  este valor varia de classe para classe.
 */

int Block::calcTree(ST::SymbolTable *scope){
	int value = 0;
	HasBlock *tmp = nullptr;
	for(auto& line : _lines){
		// Gambiarra para resolver problema da cópia
		//  do bloco de funções [explicado acima]
		tmp = HasBlock::cast(line);
		if(tmp != nullptr)
			tmp->setStPrevious(getScope());

		line->setReturning(false);
		value = line->calcTree(getScope());

		if(line->getNodeType() == AST::return_nt || line->isReturning()){
			setType(line->getType());
			setReturning(true);
			getScope()->removeRefs();
			return value;//retorna o valor do 1* 'return' encontrado
		}
	}
	setReturning(false);
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

	if(index != nullptr && stype != Types::arr_t){
		setType(Types::unknown_t);
		Errors::throwErr(Errors::attempt_index, getId(), Types::mascType[stype]);
	}else if(wasCalledLikeFunc() && stype != Types::func_t){
		setType(Types::unknown_t);
		Errors::throwErr(Errors::attempt_call, getId(), Types::mascType[stype]);
	}

	if(index != nullptr){
		return _calcArrVal(scope, symbol);
	}else if(wasCalledLikeFunc()){
		return _calcFuncVal(scope, symbol);
	}else{
		setType(stype);
		return svalue;
	}
}

int Variable::_calcArrVal(ST::SymbolTable *scope, ST::Symbol *symbol){
	setType(Types::unknown_t);

	auto index = getIndex();
	int iv = index->calcTree(scope);

	if(index->getType() != Types::int_t)
		Errors::throwErr(Errors::index_wrong_type, Types::mascType[index->getType()]);

	auto arr = arrtab.getArray(symbol->getValue());
	if(arr == nullptr) return 0;

	auto val = arr->getValue(iv);
	int v = 0;

	if(val != nullptr){
		v = val->calcTree(scope);
		setType(val->getType());
	}
	return v;
}

int Variable::_calcFuncVal(ST::SymbolTable *scope, ST::Symbol *symbol){
	setType(Types::unknown_t);

	auto exprs = getParams();//expressões
	auto func = functab.getFunc(symbol->getValue())
						->getFunc();

	// cria uma copia do bloco
	auto block = Block::cast(func->getBlock())->copy();
	auto st = block->getScope();
	int tmp = 0;

	try{
		// add params na st
		auto dparams = Variable::cast(func->getParams());//parametros declarados
		while(dparams != nullptr){
			if(exprs != nullptr){
				tmp = exprs->calcTree(scope);
				st->setValue(dparams->getId(), tmp, exprs->getType());
				exprs = exprs->getNext();
			}else{
				st->setValue(dparams->getId(), 0, Types::unknown_t);
			}
			dparams = Variable::cast(dparams->getNext());
		}

		// executar bloco
		tmp = block->calcTree(scope);

		// funções só retornam inteiros, booleanos ou desconhecido
		if(block->getType() == Types::func_t ||
				block->getType() == Types::arr_t){
			Errors::throwErr(Errors::func_type_not_allowed,
					Types::mascType[block->getType()]);
		}

		setType(block->getType());
		delete block;
	}catch(int e){
		// Gambiarra para deletar a cópia do bloco
		//  mesmo que de algum erro no calc dos
		//  parametros ou do bloco
		delete block;
		throw e;
	}

	// retornar resultado
	return tmp;
}

int Value::calcTree(ST::SymbolTable *scope){
	int value = atoi(_n.c_str());
	if(_n=="true") value = 1;
	else if(_n=="false") value = 0;
	else if(_n == "nil") value = 0;

	return value;
}

int Function::calcTree(ST::SymbolTable *scope){
	if(getBlock() != nullptr){
		// Cria um novo simbolo para esta função
		FT::Symbol* symbol = functab.createFunc(this);
		return symbol->getAddr();
	}
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
			Errors::throwErr(Errors::arr_type_not_allowed,
					Types::mascType[tmp->getType()]);
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

/**
 * Cuida da atribuição a arranjos
 */
int BinOp::_calcAssignArr(ST::SymbolTable *scope, Types::Type rtype, int rv){
	auto var = Variable::cast(getLeft());
	auto index = var->getIndex();
	int iv = index->calcTree(scope);

	if(rtype == Types::arr_t)
		Errors::throwErr(Errors::arr_type_not_allowed,
				Types::mascType[rtype]);

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
	if(cond == nullptr) return 0;

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

	if(block == nullptr || cond == nullptr) return 0;

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
