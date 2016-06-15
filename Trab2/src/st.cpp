/*
 * st.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "st.hpp"
#include "ast.hpp"

using namespace ST;

// SYMBOL


// SYMBOLTABLE

// other funcs
bool SymbolTable::checkId(std::string id, bool creation/*=false*/){
	bool result = _entryList.find(id) != _entryList.end();
	//  Caso seja a declaração de uma variavel
	// só é preciso checar o primeiro nivel do escopo
	if(creation)
		return result;
	return result ? result :
		(_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

void SymbolTable::addSymbol(std::string id, Symbol *newsymbol){
	_entryList[id] = newsymbol;
}

AST::Node* SymbolTable::declVar(AST::Node *varlist, AST::Node *exprlist){
	if(exprlist == nullptr){
		return _newVar(varlist);
	}else{
		_newVar(varlist);
		return assignVar(varlist, exprlist);
	}
}

AST::Node* SymbolTable::_newVar(AST::Node *varlist){
	auto tmp = AST::Variable::cast(varlist);
	while(tmp != nullptr){
		if(checkId(tmp->getId(), true)){
			Errors::print(Errors::redefinition, tmp->getId());
			tmp->setError(true);
		}else{
			auto symbol = new Symbol(Types::unknown_t);
			addSymbol(tmp->getId(), symbol);
		}
		tmp->setUse(AST::decl_u);
		tmp = AST::Variable::cast(tmp->getNext());
	}
	return varlist;
}

AST::Node* SymbolTable::assignVar(AST::Node *varlist, AST::Node *exprlist){
	AST::Node *retlist = nullptr;
	AST::Node *tmp = nullptr;
	auto var = AST::Variable::cast(varlist);
	auto expr = exprlist;
	while(var != nullptr){
		tmp = var;
		if(expr != nullptr){
			tmp = _assignVar(var,expr);
			expr = expr->getNext();
			exprlist->setNext(nullptr);
			exprlist = expr;
		}
		var = AST::Variable::cast(var->getNext());
		varlist->setNext(nullptr);
		varlist = var;

		if(retlist==nullptr) retlist = tmp;
		else retlist->setNext(tmp);
	}

	return retlist;
}

AST::Node* SymbolTable::_assignVar(AST::Variable *var, AST::Node *expr){
	if(!checkId(var->getId(), false)){
		Errors::print(Errors::without_declaration, var->getId());
	}else if(!var->getError()){
		auto symbol = getSymbol(var->getId());
		if(expr != nullptr){
			//symbol->setValue(expr->calcTree(this));
			symbol->setType(expr->getType());
			var->setType(expr->getType());
		}
	}
	var->setUse(AST::attr_u);
	return new AST::BinOp(var,Ops::assign,expr);
}

AST::Node* SymbolTable::useVar(std::string id, AST::Node *index){
	Types::Type type = Types::unknown_t;
	bool error = false;
	auto symbol = getSymbol(id);
	// Falta checar o index!
	if(symbol != nullptr){
		type = symbol->getType();
	}else{
		Errors::print(Errors::without_declaration, id.c_str());
		error = true;
	}

	auto var = new AST::Variable(id,index,AST::read_u,type,nullptr);
	var->setError(error);
	return var;
}

// getters
Symbol* SymbolTable::getSymbol(std::string id){
	bool result = _entryList.find(id) != _entryList.end();
	if(result)
		return _entryList[id];
	else if(_previous != nullptr)
		return _previous->getSymbol(id);
	else
		return nullptr;
}
