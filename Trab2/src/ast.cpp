/*
 * ast.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "../include/ast.hpp"

using namespace AST;

// destructors
Node::~Node(){
  if(_next != nullptr)
    delete _next;
}

Block::~Block(){
  for(auto& line : _lines)
    delete line;
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

// setters
void Node::setError(bool err){
	_error = err;
	if(err)
		setType(Types::unknown_t);
}

// virtual funcs
const char* Node::getTypeTxt(bool masc){
	if(masc)
		return Types::mascType[_type];
	else
		return Types::femType[_type];
}

