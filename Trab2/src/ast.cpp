/*
 * ast.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "../include/ast.hpp"

#include "../include/st.hpp"

using namespace AST;

// destructors
Node::~Node(){
  if(_next != nullptr)
    delete _next;
}

Block::~Block(){
  /*for(auto& line : _lines)
    delete line;*/
	delete _scope;
}

Variable::~Variable(){
	if(_index != nullptr)
		delete _index;
	if(_params != nullptr)
		delete _params;
}

Array::~Array(){
  if(_values != nullptr)
    delete _values;
}

Function::~Function(){
  if(_params != nullptr)
    delete _params;
  if(_block != nullptr)
    delete _block;
}

Return::~Return(){
  if(_expr != nullptr)
    delete _expr;
}

BinOp::~BinOp(){
  if(_left != nullptr)
    delete _left;
  if(_right != nullptr)
    delete _right;
}

UniOp::~UniOp(){
  if(_right != nullptr)
    delete _right;
}

CondExpr::~CondExpr(){
	if(_cond != nullptr)
		delete _cond;
	if(_thenBranch != nullptr)
		delete _thenBranch;
	if(_elseBranch != nullptr)
		delete _elseBranch;
}

WhileExpr::~WhileExpr(){
	if(_cond != nullptr)
		delete _cond;
	if(_block != nullptr)
		delete _block;
}

// other funcs
Block* Block::copy(){
	Block *b = new Block(this->getScope()->copy());
	for(auto& line : _lines)
		b->addLine(line);

	return b;
}

// setters

// virtual funcs
/**
 * Função que retorna uma string representando o tipo
 *  do Nodo dependendo do parametro masc
 *
 * @param masc	TRUE caso queria a representação masculina do tipo
 * 				FALSE caso queria a representação feminina do tipo
 */
const char* Node::getTypeTxt(bool masc){
	if(masc)
		return Types::mascType[_type];
	else
		return Types::femType[_type];
}
