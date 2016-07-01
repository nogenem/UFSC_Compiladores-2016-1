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
	  function_nt, return_nt, binop_nt, uniop_nt, condexpr_nt, whileexpr_nt };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
public:
	// constructors
	Node():
		_type(Types::unknown_t), _next(nullptr), _returning(false){}
	Node(Types::Type type, Node *next):
		_type(type), _next(next), _returning(false){}
	// destructors
	virtual ~Node();

	// virtual funcs
	virtual int calcTree(ST::SymbolTable *scope){return 0;}
	virtual NodeType getNodeType(){return node_nt;}
	virtual const char* getTypeTxt(bool masc);

	// getters
	Types::Type getType(){return _type;}
	Node* getNext(){return _next;}
	bool isReturning(){return _returning;}
	// setters
	void setType(Types::Type type){_type=type;}
	void setNext(Node *next){_next=next;}
	void setReturning(bool value){_returning=value;}
protected:
	Types::Type _type;
	Node* _next;
	// Gambiarra para saber que IF e WHILE
	//  tão retornando...
	bool _returning;
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

	// getters
	ST::SymbolTable* getScope(){return _scope;}
protected:
	ST::SymbolTable *_scope;
	NodeList _lines;
};

class Variable : public Node {
public:
	// constructors
	Variable(std::string id, Node *index, Node *params, Types::Type type, Node *next):
		_id(id), _index(index), _params(params), Node(type,next){}
	// destructors
	~Variable();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return variable_nt;}

	// static funcs
	static Variable* cast(Node *node);

	// getters
	const char* getId(){return _id.c_str();}
	Node* getIndex(){return _index;}
	Node* getParams(){return _params;}
	// setters
	void setId(std::string id){_id=id;}
	void setIndex(Node *index){_index=index;}
	void setParams(Node *params){_params=params;}
protected:
	std::string _id;
	Node *_index;
	Node *_params;
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

	// getters
	Node* getParams(){return _params;}
	Node* getBlock(){return _block;}
	// setters

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

	// getters
	Node* getValues(){return _values;}
protected:
	Node *_values;
};

class Return : public Node {
public:
	// constructors
	Return(Node *expr):
		_expr(expr),
		Node(expr!=nullptr?expr->getType():Types::unknown_t,
			 nullptr){}
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
	Ops::Operation getOp(){return _op;}
	Node* getRight(){return _right;}
	// setters
	void setLeft(Node *node){_left=node;}
	void setOp(Ops::Operation op){_op=op;}
	void setRight(Node *node){_right=node;}
protected:
	int _calcAssignArr(ST::SymbolTable *scope, Types::Type rtype, int rv);
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
	Ops::Operation getOp(){return _op;}
	// setters
	void setOp(Ops::Operation op){_op=op;}
	void setRight(Node *node){_right=node;}
protected:
	Ops::Operation _op;
	Node *_right;
};

class CondExpr : public Node {
public:
	// constructors
	CondExpr(Node *cond, Node *thenBranch, Node *elseBranch):
		_cond(cond), _thenBranch(Block::cast(thenBranch)),
		_elseBranch(Block::cast(elseBranch)){}

	// destructors
	~CondExpr();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return condexpr_nt;}

	// static funcs
	static CondExpr* cast(Node *node);

	// getters
	Node* getCond(){return _cond;}
	Block* getThenBranch(){return _thenBranch;}
	Block* getElseBranch(){return _elseBranch;}
	// setters

private:
	Node *_cond;
	Block *_thenBranch, *_elseBranch;
};

class WhileExpr : public Node {
public:
	// constructors
	WhileExpr(Node *cond, Node *block):
		_cond(cond), _block(Block::cast(block)){}

	// destructors
	~WhileExpr();

	// virtual funcs
	int calcTree(ST::SymbolTable *scope);
	NodeType getNodeType(){return whileexpr_nt;}

	// static funcs
	static WhileExpr* cast(Node *node);

	// getters
	Node* getCond(){return _cond;}
	Block* getBlock(){return _block;}
	// setters

private:
	Node *_cond;
	Block *_block;
};

}
