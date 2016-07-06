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
	if(_scope != nullptr)
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

// virtual funcs

/**
 * Função responsavel por alterar o ponteiro da ST
 *  'previous' do bloco da função.
 *
 * @param prev	Nova ST que sera a 'previous' da ST
 * 				 do bloco da função.
 */
void Function::setStPrevious(ST::SymbolTable *prev){
	auto st = Block::cast(getBlock())->getScope();
	st->setPrevious(prev);
}

/**
 * Função responsavel por alterar o ponteiro da ST
 *  'previous' do bloco THEN e ELSE do IF.
 *
 * @param prev	Nova ST que sera a 'previous' da ST
 * 				 do bloco THEN e ELSE.
 */
void CondExpr::setStPrevious(ST::SymbolTable *prev){
	auto st = Block::cast(getThenBranch())->getScope();
	st->setPrevious(prev);
	if(getElseBranch() != nullptr){
		st = Block::cast(getElseBranch())->getScope();
		st->setPrevious(prev);
	}
}

/**
 * Função responsavel por alterar o ponteiro da ST
 *  'previous' do bloco do WHILE.
 *
 * @param prev	Nova ST que sera a 'previous' da ST
 * 				 do bloco do WHILE.
 */
void WhileExpr::setStPrevious(ST::SymbolTable *prev){
	auto st = Block::cast(getBlock())->getScope();
	st->setPrevious(prev);
}

// other funcs

/**
 * Função que cria uma cópia do bloco.
 * Ela cria um novo bloco passando uma nova cópia
 *  da sua própria ST e então copia todas as suas
 *  'linhas' para o novo bloco.
 */
Block* Block::copy(){
	Block *b = new Block(this->getScope()->copy());
	for(auto& line : _lines){
		line->setReturning(false);
		b->addLine(line);
	}
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
