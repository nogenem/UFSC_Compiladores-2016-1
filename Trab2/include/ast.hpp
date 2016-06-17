/*
 * ast.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <string>
#include <vector>

#include "util.hpp"

namespace ST {
class SymbolTable;
} /* namespace ST */

namespace AST {

// Possiveis tipos de nodos
enum NodeType { node_nt, block_nt, value_nt, variable_nt, array_nt,
	  function_nt, return_nt, binop_nt, uniop_nt };

// Possiveis usos de uma variavel
enum Use { unknown_u, attr_u, decl_u, param_u, read_u };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
public:
	// constructors
	Node():
		_type(Types::unknown_t), _next(nullptr), _error(false){}
	Node(Types::Type type, Node *next):
		_type(type), _next(next), _error(false){}
	// destructors
	virtual ~Node();

	// virtual funcs
	virtual int calcTree(ST::SymbolTable *scope){return 0;}
	virtual NodeType getNodeType(){return node_nt;}
	virtual const char* getTypeTxt(bool masc);

	// getters
	const Types::Type getType(){return _type;}
	Node* getNext(){return _next;}
	bool getError(){return _error;}
	// setters
	void setType(Types::Type type){_type=type;}
	void setNext(Node *next){_next=next;}
	void setError(bool err){_error=err;}
protected:
	Types::Type _type;
	Node* _next;
	bool _error;
};

class Block : public Node {
public:
	// constructors
	Block(ST::SymbolTable *st):
		_scope(st){}
	// destructors
	~Block();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return block_nt;}

	// static funcs
	static Block* cast(Node *node);

	// list funcs
	void addLine(Node *line){_lines.push_back(line);}
protected:
	ST::SymbolTable *_scope;
	NodeList _lines;
};

class Variable : public Node {
public:
	// constructors
	Variable(std::string id, Node *extra, Use use, Types::Type type, Node *next):
		_id(id), _extra(extra), _use(use), Node(type,next){}
	// destructors
	~Variable();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return variable_nt;}

	// static funcs
	static Variable* cast(Node *node);

	// getters
	const char* getId(){return _id.c_str();}
	Node* getExtra(){return _extra;}
	const Use getUse(){return _use;}
	// setters
	void setId(std::string id){_id=id;}
	void setExtra(Node *extra){_extra=extra;}
	void setUse(Use use){_use=use;}
protected:
	std::string _id;
	// pode ser o index do arranjo ou
	//  os parametros da função
	Node *_extra;// por falta de nome melhor...
	Use _use;
};

class Value : public Node {
public:
	// constructors
	Value(std::string n, Types::Type type):
		_n(n), Node(type,nullptr){}
	// destructors
	virtual ~Value(){}

	// virtual funcs
	virtual int calcTree(ST::SymbolTable *scope);
	virtual NodeType getNodeType(){return value_nt;}

	// static funcs
	static Value* cast(Node *node);

	// getters
	const char* getN(){return _n.c_str();}
	// setters
	void setN(std::string n){_n=n;}
protected:
	std::string _n;
};

class Function : public Value {
public:
	// constructors
	Function(Node *params, Node *block):
		_params(params), _block(block),
		Value("",Types::func_t){}
	// destructors
	~Function();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return function_nt;}

	// static funcs
	static Function* cast(Node *node);
protected:
	Node *_params;
	Node *_block;
};

class Array : public Value {
public:
	// constructors
	Array(Node *values):
		_values(values),
		Value("",Types::arr_t){}
	// destructors
	~Array();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return array_nt;}

	// static funcs
	static Array* cast(Node *node);
protected:
	Node *_values;
};

class Return : public Node {
public:
	// constructors
	Return(Node *expr):
		_expr(expr),
		Node(expr->getType(),nullptr){}
	// destructors
	~Return();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return return_nt;}

	// static funcs
	static Return* cast(Node *node);

	// getters
	Node* getExpr(){return _expr;}
	// setters
	void setExpr(Node *expr){_expr=expr;}
protected:
	Node *_expr;
};

class BinOp : public Node{
public:
	// constructors
	BinOp(Node *left, Ops::Operation op, Node *right):
		_left(left), _op(op), _right(right){}
	// destructors
	~BinOp();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return binop_nt;}

	// static funcs
	static BinOp* cast(Node *node);

	// getters
	Node* getLeft(){return _left;}
	const Ops::Operation getOp(){return _op;}
	Node* getRight(){return _right;}
	// setters
	void setLeft(Node *node){_left=node;}
	void setOp(Ops::Operation op){_op=op;}
	void setRight(Node *node){_right=node;}
protected:
	Node *_left;
	Ops::Operation _op;
	Node *_right;
};

class UniOp : public Node{
public:
	// constructors
	UniOp(Ops::Operation op, Node *right):
		_op(op), _right(right){}
	// destructors
	~UniOp();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return uniop_nt;}

	// static funcs
	static UniOp* cast(Node *node);

	// getters
	Node* getRight(){return _right;}
	const Ops::Operation getOp(){return _op;}
	// setters
	void setOp(Ops::Operation op){_op=op;}
	void setRight(Node *node){_right=node;}
protected:
	Ops::Operation _op;
	Node *_right;
};

}
