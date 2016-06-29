/*
 * castast.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "ast.hpp"

using namespace AST;

Block* Block::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == block_nt)
    return dynamic_cast<Block*>(node);
  else
    return nullptr;
}

Variable* Variable::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == variable_nt)
    return dynamic_cast<Variable*>(node);
  else
    return nullptr;
}

Value* Value::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == value_nt)
    return dynamic_cast<Value*>(node);
  else
    return nullptr;
}

Function* Function::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == function_nt)
    return dynamic_cast<Function*>(node);
  else
    return nullptr;
}

Array* Array::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == array_nt)
    return dynamic_cast<Array*>(node);
  else
    return nullptr;
}

Return* Return::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == return_nt)
    return dynamic_cast<Return*>(node);
  else
    return nullptr;
}

BinOp* BinOp::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == binop_nt)
    return dynamic_cast<BinOp*>(node);
  else
    return nullptr;
}

UniOp* UniOp::cast(Node *node){
  if(node != nullptr &&
      node->getNodeType() == uniop_nt)
    return dynamic_cast<UniOp*>(node);
  else
    return nullptr;
}

CondExpr* CondExpr::cast(Node *node){
	if(node != nullptr &&
			node->getNodeType() == condexpr_nt)
		return dynamic_cast<CondExpr*>(node);
	else
		return nullptr;
}

WhileExpr* WhileExpr::cast(Node *node){
	if(node != nullptr &&
			node->getNodeType() == whileexpr_nt)
		return dynamic_cast<WhileExpr*>(node);
	else
		return nullptr;
}
