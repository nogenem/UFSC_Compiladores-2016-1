/*
 * itr.hpp
 *
 *  Created on: 17 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include "util.hpp"

namespace AST {
class Node;
} /* namespace AST */
namespace ST {
class SymbolTable;
} /* namespace ST */

extern ST::SymbolTable *symtab;

/* Interpreter */
namespace ITR {

	void execExpr(AST::Node *node, bool print=false);

	void _print(int value, Types::Type type);

}
