/*
 * ft.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <map>
#include "ast.hpp"

namespace FT {
	static std::map<int,AST::Function*> _table;
	static int _funcN=0;//numero de funcs

	int callFunc(int funcAddr, AST::Node *params);
	void addFunc(AST::Node *func);
}
