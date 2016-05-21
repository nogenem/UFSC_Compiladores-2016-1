#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "st.hpp"
#include "util.hpp"

namespace AST {

// Possiveis usos de variaveis, arrays e funções
enum Use { attr, declr, def, read, param, comp };

// Possiveis tipos de nodos
enum NodeType { node_nt, block_nt, value_nt, variable_nt, array_nt,
  function_nt, return_nt, binop_nt, uniop_nt, condexpr_nt, whileexpr_nt,
  composite_type_nt };

class Node;

typedef std::vector<Node*> NodeList; //List of ASTs

class Node {
  public:
    Node():
      next(nullptr),type(Types::unknown_t){}
    Node(Types::Type type):
      next(nullptr),type(type){}
    Node(Node *next, Types::Type type):
      next(next),type(type){}

    virtual ~Node() {}
    virtual void printTree(){}
    virtual NodeType getNodeType(){return node_nt;}

    // Next nodo, usado em variaveis,
    // arrays e lista de parametros para funções
    Node *next;
    // Tipo do nodo
    Types::Type type;
};

class Block : public Node{
  public:
    Block(ST::SymbolTable *symtab):
      symtab(symtab){}
    Block(): symtab(nullptr) {}

    void printTree();
    NodeType getNodeType(){return block_nt;}

    // Lista de linhas do block
    NodeList lines;
    // 'Escopo'
    ST::SymbolTable *symtab;
};

class Value : public Node {
  public:
    Value(std::string n, Types::Type type):
      n(n), Node(type){}

    void printTree();
    NodeType getNodeType(){return value_nt;}

    // Valor [numero,TRUE,FALSE]
    std::string n;
};

class Variable : public Node {
  public:
    Variable(std::string id, Node *next, Use use,
      Types::Type type=Types::unknown_t):
      id(id), use(use), Node(next,type){}

    void printTree();
    virtual NodeType getNodeType(){return variable_nt;}

    void setType(Types::Type t){type=t;}
    void setCompType(std::string id){compType=id;type=Types::composite_t;}
    const char* getTypeTxt(bool masc);
    virtual Kinds::Kind getKind(){return Kinds::variable_t;}
    virtual bool equals(Variable *var, bool checkNext=false);

    std::string id;
    // Para que a variavel vai ser usada
    Use use;
    // Tipo composto
    std::string compType="";
};

class Array : public Variable {
  public:
    Array(std::string id, Node *next, Use use):
      Array(id,next,nullptr,use,1,Types::unknown_t){}
    Array(std::string id, Node *index, Use use, Types::Type type):
      Array(id,nullptr,index,use,1,type){}
    Array(std::string id, Node *next, Use use,
      int aSize, Types::Type type):
      Array(id,next,nullptr,use,aSize,type){}
    Array(std::string id, Node *next, Node *i,
      Use use, int aSize, Types::Type type);

    void printTree();
    NodeType getNodeType(){return array_nt;}

    void setSize(int n);
    Kinds::Kind getKind(){return Kinds::array_t;}
    bool equals(Variable *var, bool checkNext=false);

    // Indice do arranjo
    Node *index;
    // Tamanho do arranjo
    int size=1;
};

class Function : public Variable {
  public:
    Function(std::string id, Node *params, Node *block, Use use,
      Types::Type type=Types::unknown_t);

    void printTree();
    NodeType getNodeType(){return function_nt;}

    Kinds::Kind getKind(){return Kinds::function_t;}
    bool equals(Variable *var, bool checkNext=false);

    // Parametros da função, tanto na declaração,
    // definição e uso da função
    Node *params;
    // Corpo da função
    Node *block;
};

class CompositeType : public Node {
  public:
    CompositeType(std::string id, Node *block):
      id(id), block(block){}

    void printTree();
    NodeType getNodeType(){return composite_type_nt;}

    Kinds::Kind getKind(){return Kinds::type_t;}

    std::string id;
    Node *block;
};

class Return : public Node {
  public:
    Return(Node *expr):
      expr(expr),Node(expr->type){}

    void printTree();
    NodeType getNodeType(){return return_nt;}

    // Expresão após o 'return'
    Node *expr;
    // Tipo da função aonde este 'return' esta
    // (usado para verificar se o retorno esta correto)
    Types::Type funcType=Types::unknown_t;
};

class CondExpr : public Node {
  public:
    CondExpr(Node *cond, Node *thenBranch, Node *elseBranch);

    void printTree();
    NodeType getNodeType(){return condexpr_nt;}

    // Condição, ramo then e ramo else do IF
    Node *cond, *thenBranch, *elseBranch;
};

class WhileExpr : public Node {
  public:
    WhileExpr(Node *cond, Node *block);

    void printTree();
    NodeType getNodeType(){return whileexpr_nt;}

    // Condição e corpo do While
    Node *cond, *block;
};

class BinOp : public Node {
  public:
    BinOp(Node *left, Ops::Operation op, Node *right);

    void printTree();
    NodeType getNodeType(){return binop_nt;}

    // Lado esquerdo e direito da operação binaria
    Node *left, *right;
    // Operação
    Ops::Operation op;
};

class UniOp : public Node {
  public:
    UniOp(Ops::Operation op, Node *right);

    void printTree();
    NodeType getNodeType(){return uniop_nt;}

    // Lado direito da operação unária
    Node *right;
    // Operação
    Ops::Operation op;
};

}
