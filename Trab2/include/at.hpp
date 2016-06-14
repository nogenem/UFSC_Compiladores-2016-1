/*
 * at.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <map>
#include "ast.hpp"

namespace AT {
	typedef std::map<int,int> arrValues;
	static std::map<int,arrValues> _table;
	static int _arrN=0;//numero de arrays

	int getValue(int arrAddr, int index);
	void setValue(int arrAddr, int index, int value);

	void createArray(AST::Node *values);
}
