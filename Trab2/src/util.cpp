/*
 * util.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "util.hpp"
#include <stdio.h>      /* printf */
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

extern int yylineno;

using namespace Types;

Type Types::unType(Ops::Operation op, Type right){
	auto rtxt = mascType[right];
	Type ret;
	switch (op) {
		case Ops::u_minus:
			ret = int_t;
			if(right != int_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
				ret = unknown_t;
			}
			break;
		case Ops::u_not:
			ret = bool_t;
			if(right != bool_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], rtxt);
				ret = unknown_t;
			}
			break;
		case Ops::u_paren:
			ret = right;
			break;
		default: ret = unknown_t;
	}
	return ret;
}

Type Types::binType(Type left, Ops::Operation op, Type right){
	auto ltxt = mascType[left];
	auto rtxt = mascType[right];
	Type ret;
	switch (op) {
		case Ops::assign:
			ret = left;
			break;
		case Ops::plus:
		case Ops::b_minus:
		case Ops::times:
		case Ops::division:
			ret = int_t;
			if(left != int_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], ltxt);
				ret = unknown_t;
			}
			if(right != int_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
				ret = unknown_t;
			}
			break;

		case Ops::eq:
		case Ops::neq:
		case Ops::grt:
		case Ops::grteq:
		case Ops::lst:
		case Ops::lsteq:
			ret = bool_t;
			if(left != int_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], ltxt);
				ret = unknown_t;
			}
			if(right != int_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
				ret = unknown_t;
			}
			break;

		case Ops::b_and:
		case Ops::b_or:
			ret = bool_t;
			if(left != bool_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], ltxt);
				ret = unknown_t;
			}
			if(right != bool_t){
				Errors::print(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], rtxt);
				ret = unknown_t;
			}
			break;

		default: ret = unknown_t;
	}

	return ret;
}

using namespace Errors;

// ERRORS
void Errors::print(ErrorType error, ...){
	va_list ap;
	va_start(ap, error);
	fprintf(stderr, "[Linha %d] Erro ", yylineno);
	vfprintf(stderr, messages[error], ap);
	fprintf(stderr, "\n");
}
