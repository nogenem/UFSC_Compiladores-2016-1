#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "util.hpp"

namespace AST {

// Possiveis tipos de nodos
enum NodeType { node_nt, block_nt, value_nt, variable_nt, array_nt,
  function_nt, return_nt, binop_nt, uniop_nt, condexpr_nt, whileexpr_nt,
  composite_type_nt };

// Possiveis usos de variaveis, arrays e funções
enum Use { unknown_u, attr_u, declr_u, def_u, param_u, comp_u, read_u, read_comp_u };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
public:
  // constructors
  Node():
    Node(nullptr, Types::unknown_t){}
  Node(Node *next, Types::Type type):
    _next(next), _type(type){}

  // destructors
  virtual ~Node();

  // other funcs
  virtual void printTree(){}
  virtual NodeType getNodeType(){return node_nt;}

  // getters
  Node* getNext(){return _next;}
  virtual Types::Type getType(bool checkComps=false);
  virtual const char* getTypeTxt(bool masc);
  // setters
  void setNext(Node *next){_next=next;}
  void setType(Types::Type type){_type=type;}
protected:
  // Proximo nodo, usado na declaração de variaveis,
  //  lista de parametros e afins
  Node *_next;
  // Tipo do nodo
  Types::Type _type;
};

class Block : public Node {
public:
  // constructors
  Block(){}

  // destructors
  ~Block();

  // other funcs
  void printTree();
  NodeType getNodeType(){return block_nt;}

  // static funcs
  static Block* cast(Node *node);

  // list funcs
  void addLine(Node *line);

  // getters
  NodeList& getLines(){return _lines;}
protected:
  // Lista de linhas do bloco
  NodeList _lines;
};

class Value : public Node {
public:
  // constructors
  Value(std::string n, Types::Type type):
    Value(n,nullptr,type){}
  Value(std::string n, Node *next, Types::Type type):
    _n(n), Node(next,type){}

  // other funcs
  void printTree();
  NodeType getNodeType(){return value_nt;}

  // static funcs
  static Value* cast(Node *node);

  // getters
  const char* getN(){return _n.c_str();}
protected:
  // valor [numero, TRUE, FALSE]
  std::string _n;
};

class Variable : public Node {
public:
  // constructors
  // Construtor dumb usado para gambiarra no parser.y
  Variable(std::string id, Types::Type type):
    _id(id), _use(unknown_u), _compType(""), _nextComp(nullptr),
    Node(nullptr,type){}

  Variable(std::string id, Use use, std::string compType, Node *nextComp,
    Node *next, Types::Type type=Types::unknown_t):
    _id(id), _use(use), _compType(compType), _nextComp(nextComp),
    Node(next,type){}

  // other funcs
  virtual void printTree();
  virtual NodeType getNodeType(){return variable_nt;}
  virtual Kinds::Kind getKind(){return Kinds::variable_t;}
  virtual bool equals(Variable *var, bool checkNext=false);
  bool hasCompType();

  // static funcs
  static Variable* cast(Node *node);

  // getters
  const char* getId(){return _id.c_str();}
  Use getUse(){return _use;}
  std::string getCompType(){return _compType;}
  Node* getNextComp(){return _nextComp;}
  Types::Type getType(bool checkComps=false);
  const char* getTypeTxt(bool masc);
  // setters
  void setId(std::string id){_id=id;}
  void setUse(Use use){_use=use;}
  void setCompType(std::string compType){_compType=compType;}
  void setNextComp(Node *node){_nextComp=node;}
  void setType(Types::Type type, std::string compType);
protected:
  // Identificador da variavel
  std::string _id;
  // Aonde ela esta sendo usada
  Use _use;
  // Tipo complexo da variavel
  std::string _compType;
  // Proximo componente [p.x.y]
  Node *_nextComp;
};

class Array : public Variable {
public:
  // constructors
  // Construtor dumb usado para gambiarra no parser.y
  Array(Node *index, std::string id, Types::Type type):
      _index(index), Variable(id, type){}

  Array(Node *index, int size, std::string id, Use use,
    std::string compType, Node *nextComp, Node *next,
    Types::Type type=Types::unknown_t);

  // destructors
  ~Array();

  // other funcs
  void printTree();
  NodeType getNodeType(){return array_nt;}
  Kinds::Kind getKind(){return Kinds::array_t;}
  bool equals(Variable *var, bool checkNext=false);

  // static funcs
  static Array* cast(Node *node);

  // getters
  Node* getIndex(){return _index;}
  int getSize(){return _size;}
  // setters
  void setIndex(Node *index){_index=index;}
  void setSize(int size);
protected:
  // Indice do arranjo (a[i+2] := 5;)
  Node *_index;
  // Tamanho do arranjo (int[2]: a;)
  int _size;
};

class Function : public Variable {
public:
  // constructors
  Function(Node *params, Node *block, std::string id, Use use,
    std::string compType, Node *nextComp, Node *next,
    Types::Type type=Types::unknown_t);

  // destructors
  ~Function();

  // other funcs
  void printTree();
  NodeType getNodeType(){return function_nt;}
  Kinds::Kind getKind(){return Kinds::function_t;}
  bool equals(Variable *var, bool checkNext=false);

  // static funcs
  static Function* cast(Node *node);

  // getters
  Node* getParams(){return _params;}
  Node* getBlock(){return _block;}
  // setters
  void setParams(Node *params){_params=params;}
  void setBlock(Node *block){_block=block;}
protected:
  // Parametros da função
  Node *_params;
  // Corpo da função
  Node *_block;
};

class CompositeType : public Node {
public:
  // constructors
  CompositeType(std::string id, Node *block):
    _id(id), _block(block){}

  // other funcs
  void printTree();
  NodeType getNodeType(){return composite_type_nt;}
  Kinds::Kind getKind(){return Kinds::type_t;}

  // static funcs
  static CompositeType* cast(Node *node);

  // getters
  const char* getId(){return _id.c_str();}
  Node* getBlock(){return _block;}
  // setters
  void setId(std::string id){_id=id;}
  void setBlock(Node *block){_block=block;}
protected:
  // Identificador do tipo composto
  std::string _id;
  // Corpo do tipo composto
  Node *_block;
};

class Return : public Node {
public:
  // constructors
  Return(Node *expr):
    _expr(expr), _funcType(Types::unknown_t),
    Node(nullptr,expr->getType()){}

  // destructors
  ~Return();

  // other funcs
  void printTree();
  NodeType getNodeType(){return return_nt;}

  // static funcs
  static Return* cast(Node *node);

  // getters
  Node* getExpr(){return _expr;}
  Types::Type getFuncType(){return _funcType;}
  // setters
  void setExpr(Node *expr){_expr=expr;}
  void setFuncType(Types::Type funcType){_funcType=funcType;}
protected:
  // Expressão do retorno
  Node *_expr;
  // Tipo da função aonde este 'return' esta
  // Usado para verificar se o retorno esta correto
  Types::Type _funcType;
};

class CondExpr : public Node {
public:
  // constructors
  CondExpr(Node *cond, Node *thenBranch, Node *elseBranch);

  // destructors
  ~CondExpr();

  // other funcs
  void printTree();
  NodeType getNodeType(){return condexpr_nt;}

  // static funcs
  static CondExpr* cast(Node *node);

  // getters
  Node* getCond(){return _cond;}
  Node* getThenBranch(){return _thenBranch;}
  Node* getElseBranch(){return _elseBranch;}
  // setters
  void setCond(Node *cond){_cond=cond;}
  void setThenBranch(Node *thenBranch){_thenBranch=thenBranch;}
  void setElseBranch(Node *elseBranch){_elseBranch=elseBranch;}
protected:
  // Condição, ramo then e ramo else do IF
  Node *_cond, *_thenBranch, *_elseBranch;
};

class WhileExpr : public Node {
public:
  // constructors
  WhileExpr(Node *cond, Node *block);

  // destructors
  ~WhileExpr();

  // other funcs
  void printTree();
  NodeType getNodeType(){return whileexpr_nt;}

  // static funcs
  static WhileExpr* cast(Node *node);

  // getters
  Node* getCond(){return _cond;}
  Node* getBlock(){return _block;}
  // setters
  void setCond(Node *cond){_cond=cond;}
  void setBlock(Node *block){_block=block;}
protected:
  // Condição e corpo do While
  Node *_cond, *_block;
};

class BinOp : public Node {
public:
  // constructors
  BinOp(Node *left, Ops::Operation op, Node *right);

  // destructors
  ~BinOp();

  // other funcs
  void printTree();
  NodeType getNodeType(){return binop_nt;}

  // static funcs
  static BinOp* cast(Node *node);

  // getters
  Node* getLeft(bool checkParens=false);
  Ops::Operation getOp(){return _op;}
  Node* getRight(bool checkParens=false);
  // setters
  void setLeft(Node *left){_left=left;}
  void setOp(Ops::Operation op){_op=op;}
  void setRight(Node *right){_right=right;}
protected:
  // Lado esquerdo da operação binaria
  Node *_left;
  // Operação
  Ops::Operation _op;
  // Lado direito da operação binaria
  Node *_right;
};

class UniOp : public Node {
public:
  // constructors
  UniOp(Ops::Operation op, Node *right);

  // destructors
  ~UniOp();

  // other funcs
  void printTree();
  NodeType getNodeType(){return uniop_nt;}

  // static funcs
  static UniOp* cast(Node *node);

  // getters
  Ops::Operation getOp(){return _op;}
  Node* getRight(bool checkParens=false);
  // setters
  void setOp(Ops::Operation op){_op=op;}
  void setRight(Node *right){_right=right;}
protected:
  // Operação
  Ops::Operation _op;
  // Lado direito da operação unária
  Node *_right;
};

}
