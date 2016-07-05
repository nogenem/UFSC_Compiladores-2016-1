/*
 * st.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "../include/st.hpp"

#include <utility>

#include "../include/ast.hpp"
#include "../include/at.hpp"
#include "../include/ft.hpp"

extern AT::ArrayTable arrtab;
extern FT::FuncTable functab;

using namespace ST;

// SYMBOL

// other funcs
Symbol* Symbol::copy(){
	Symbol *s = new Symbol();
	s->setValue(this->getValue(), this->getType());
	return s;
}

// setters
/**
 * Função responsavel por alterar o valor do simbolo.
 *
 * @param value		Novo valor para o simbolo
 * @param type		Tipo deste novo valor
 */
void ST::Symbol::setValue(int value, Types::Type type) {
	// Checagens para controle de
	//  referencias
	_checkArrRef(value, type);
	_checkFuncRef(value, type);

	_value = value;
	_type = type;
}

// other funcs

/**
 * Função responsavel por fazer o controle de
 *  referencias a arranjos.
 *
 * @param value		Novo valor para o simbolo
 * @param type		Tipo deste novo valor
 */
void Symbol::_checkArrRef(int value, Types::Type type){
	// Adiciona e remove referencia a arranjos
	// Pequena proteção contra memory leak
	if(_type == Types::arr_t){
		if(type != Types::arr_t)
			arrtab.minusRef(_value);
		else if(value != _value){
			arrtab.minusRef(_value);
			arrtab.plusRef(value);
		}
	}else if(type == Types::arr_t){
		arrtab.plusRef(value);
	}
}

/**
 * Função responsavel por fazer o controle de
 *  referencias a funções.
 *
 * @param value		Novo valor para o simbolo
 * @param type		Tipo deste novo valor
 */
void Symbol::_checkFuncRef(int value, Types::Type type){
	if(_type == Types::func_t){
		if(type != Types::func_t)
			functab.minusRef(_value);
		else if(value != _value){
			functab.minusRef(_value);
			functab.plusRef(value);
		}
	}else if(type == Types::func_t){
		functab.plusRef(value);
	}
}

// SYMBOLTABLE

// other funcs
/**
 * Função responsavel por verificar se uma variavel
 *  ja foi declarada.
 *
 * @param id		Id da variavel que se quer verificar
 * @param creation	TRUE se for uma checagem na hora da criação da var
 * 					FALSE caso contrario
 */
bool SymbolTable::checkId(std::string id, bool creation/*=false*/){
	bool result = _entryList.find(id) != _entryList.end();
	//  Caso seja a declaração de uma variavel
	// só é preciso checar o primeiro nivel do escopo
	if(creation)
		return result;
	return result ? result :
		(_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

/**
 * Função responsavel por adicionar um novo simbolo na tabela
 *
 * @param id		Id da variavel a qual o novo simbolo representa
 * @param newsymbol	Novo simbolo a ser adicionado
 */
void SymbolTable::addSymbol(std::string id, Symbol *newsymbol){
	_entryList[id] = newsymbol;
}

/**
 * Função responsavel por lidar com a declaração de novas variaveis
 *
 * @param varlist	Lista de variaveis
 * @param exprlist	Lista de expressões
 */
AST::Node* SymbolTable::declVar(AST::Node *varlist, AST::Node *exprlist){
	AST::Node* ret = nullptr;

	try{
		ret = _newVar(varlist);
		if(exprlist != nullptr)// Declaração com no minimo uma atribuição
			ret = assignVar(varlist, exprlist);
	}catch(int e){}

	return ret;
}

/**
 * Função responsavel pela criação dos simbolos das novas
 *  variaveis. Ela tambem faz a checagem de redefinição.
 *
 * @param varlist	Lista de variaveis
 */
AST::Node* SymbolTable::_newVar(AST::Node *varlist){
	auto tmp = AST::Variable::cast(varlist);
	while(tmp != nullptr){
		if(checkId(tmp->getId(), true)){
			Errors::throwErr(Errors::redefinition, tmp->getId());
		}else{
			auto symbol = new Symbol(Types::unknown_t);
			addSymbol(tmp->getId(), symbol);
		}
		tmp = AST::Variable::cast(tmp->getNext());
	}
	return varlist;
}

/**
 * Função que quebra as sequencias de variaveis e expressões
 *  criando assigns separados.
 *
 * @param varlist	Lista de variaveis
 * @param exprlist	Lista de expressões
 */
AST::Node* SymbolTable::assignVar(AST::Node *varlist, AST::Node *exprlist){
	AST::Node *retlist = nullptr;
	AST::Node *rettmp = nullptr;
	AST::Node *tmp = nullptr;
	auto var = AST::Variable::cast(varlist);
	auto expr = exprlist;
	// Esta parte vai quebrar as sequencias da atribuição (a,b=1,2;)
	//  em assigns separados (a=1; b=2;)
	// As variaveis rettmp e tmp são usadas para ajudar na quebra das sequencias...
	//  [a,b ==> b é next de a; logo, tenho que fazer 'a' não ter nenhum next]
	while(var != nullptr){
		tmp = var;
		if(expr != nullptr){
			tmp = new AST::BinOp(var,Ops::assign,expr);
			// Quebra a sequencia de next da expressão
			expr = expr->getNext();
			exprlist->setNext(nullptr);
			exprlist = expr;
		}
		// Quebra a sequencia de next das variaveis
		var = AST::Variable::cast(var->getNext());
		varlist->setNext(nullptr);
		varlist = var;

		// Vai sequenciando os assigns criados
		if(retlist==nullptr){
			retlist = rettmp = tmp;
		}else{
			rettmp->setNext(tmp);
			rettmp = tmp;
		}
	}
	// Retorna a sequencia de assigns
	return retlist;
}

/**
 * Função que cuida do uso de variaveis. Ela tambem faz a
 *  checagem de declaração da variavel.
 *
 * @param id		Id da variavel
 * @param index		Index da variavel, caso seja o uso de arranjo
 * @param params	Parametros da variavel, caso seja o uso de função
 */
AST::Node* SymbolTable::useVar(std::string id, AST::Node *index,
		AST::Node *params, bool calledLikeFunc/*=false*/){
	Types::Type type = Types::unknown_t;
	auto symbol = getSymbol(id);

	if(symbol != nullptr)
		type = symbol->getType();

	return new AST::Variable(id,index,params,calledLikeFunc,type,nullptr);
}

/**
 * Função que diminui em 1 a contagem de referencias de todas
 *  as variaveis locais dentro deste escopo.
 * Usada para remover as referencias ao final de um bloco.
 */
void SymbolTable::removeRefs(){
	for(auto& iter : _entryList){
		auto& symbol = iter.second;
		if(symbol->getType() == Types::arr_t ||
				symbol->getType() == Types::func_t){
			arrtab.minusRef(symbol->getValue());
		}
	}
}

SymbolTable* SymbolTable::copy(){
	SymbolTable *st = new SymbolTable(this->_previous);
	for(auto& iter : _entryList){
		auto& symbol = iter.second;
		st->addSymbol(iter.first, symbol->copy());
	}
	return st;
}

// getters
/**
 * Função que retorna o simbolo da variavel com 'id'.
 * Esta fução faz a checagem em todos os escopos 'acima'
 *  do escopo atual.
 *
 * @param id	Id da variavel que se quer o simbolo
 */
Symbol* SymbolTable::getSymbol(std::string id){
	bool result = _entryList.find(id) != _entryList.end();
	if(result)
		return _entryList[id];
	else if(_previous != nullptr)
		return _previous->getSymbol(id);
	else
		return nullptr;
}

// setters
void SymbolTable::setValue(std::string id, int value, Types::Type type){
	auto symbol = getSymbol(id);
	if(symbol != nullptr){
		symbol->setValue(value, type);
	}
}
