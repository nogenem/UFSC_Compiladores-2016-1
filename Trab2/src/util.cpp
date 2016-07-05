/*
 * util.cpp
 *
 *  Created on: 11 de jun de 2016
 *      Author: Gilne
 */

#include "../include/util.hpp"

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <stdio.h>      /* printf */

extern int yylineno;

using namespace Types;

/**
 * Função responsavel por verificar se o tipo do
 *  operando bate com o tipo da operação unária.
 *
 * @param op	Operação que se esta verificando
 * @param right	Tipo do operando
 */
Type Types::unType(Ops::Operation op, Type right){
	auto rtxt = mascType[right];
	Type ret;
	switch (op) {
		case Ops::u_minus:
			ret = int_t;
			if(right != int_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
			break;
		case Ops::u_not:
			ret = bool_t;
			if(right != bool_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], rtxt);
			break;
		case Ops::u_paren:
			ret = right;
			break;
		default: ret = unknown_t;
	}
	return ret;
}

/**
 * Função responsavel por verificar se os tipos dos
 *  operandos bate com o tipo da operação binaria.
 *
 * @param left	Tipo do operando da esquerda
 * @param op	Operação que se esta verificando
 * @param right	Tipo do operando da direita
 */
Type Types::binType(Type left, Ops::Operation op, Type right){
	auto ltxt = mascType[left];
	auto rtxt = mascType[right];
	Type ret;
	switch (op) {
		case Ops::assign:
			ret = right;
			break;
		case Ops::plus:
		case Ops::b_minus:
		case Ops::times:
		case Ops::division:
			ret = int_t;
			if(left != int_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], ltxt);
			if(right != int_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
			break;

		case Ops::eq:
		case Ops::neq:
			ret = bool_t;
			if(left != right)
				Errors::throwErr(Errors::different_types, Ops::opName[op],
						ltxt, rtxt);

			break;
		case Ops::grt:
		case Ops::grteq:
		case Ops::lst:
		case Ops::lsteq:
			ret = bool_t;
			if(left != int_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], ltxt);
			if(right != int_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[int_t], rtxt);
			break;

		case Ops::b_and:
		case Ops::b_or:
			ret = bool_t;
			if(left != bool_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], ltxt);
			if(right != bool_t)
				Errors::throwErr(Errors::op_wrong_type, Ops::opName[op],
						mascType[bool_t], rtxt);
			break;

		default: ret = unknown_t;
	}

	return ret;
}

using namespace Errors;

// ERRORS
/**
 * Função responsavel por printar os erros
 *
 * @param error	Erro que se quer printar
 * @param ...	Valores que serão usados na mensagem de erro
 */
void Errors::throwErr(ErrorType error, ...){
	va_list ap;
	va_start(ap, error);
	fprintf(stderr, "[Linha %d] Erro ", yylineno);
	vfprintf(stderr, messages[error], ap);
	fprintf(stderr, "\n");
	throw -1;
}
