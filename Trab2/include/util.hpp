/*
 * util.hpp
 *
 *  Created on: 10 de jun de 2016
 *      Author: Gilne
 */

#pragma once

#include <vector>

namespace Ops {
	enum Operation {plus, b_minus, times, division, assign, eq, neq, grt,
	  grteq, lst, lsteq, b_and, b_or, u_not, u_minus, u_paren};

	const std::vector<bool> masculineOp = {
	    false,  //soma inteira
	    false,  //subtracao inteira
	    false,  //multiplicacao inteira
	    false,  //divisao inteira
	    false,  //atribuicao intera (nunca usado)
	    true,   //igual inteiro
	    true,   //diferente inteiro
	    true,   //maior inteiro
	    true,   //maior ou igual inteiro
	    true,   //menor inteiro
	    true,   //menor ou igual inteiro
	    true,   //e booleano
	    true,   //ou booleano
	    true,   //nao booleano
	    true,   //menos unario inteiro
	    false   //parenteses inteiro (nunca usado)
	};

	const std::vector<const char*> opName = {
	  "soma", "subtracao", "multiplicacao", "divisao",
	  "atribuicao", "igual", "diferente", "maior",
	  "maior ou igual", "menor", "menor ou igual", "e",
	  "ou", "nao", "menos unario", "parenteses"
	};
}

namespace Types {
	enum Type{unknown_t, int_t, bool_t, arr_t, func_t};

	const std::vector<const char*> mascType = {
		"desconhecido",
		"inteiro",
		"booleano",
		"arranjo",
		"funcao"
	};
	const std::vector<const char*> femType = {
		"desconhecida",
		"inteira",
		"booleana",
		"arranjo",
		"funcao"
	};

	Type unType(Ops::Operation op, Type right);
	Type binType(Type left, Ops::Operation op, Type right);
}

namespace Errors {
	const std::vector<const char*> messages = {
			"syntax error",//syntax_error
			"lexico: simbolo desconhecido: %c.",//unknown_symbol
			"semantico: variavel %s sem declaracao.",//without_declaration
			"semantico: variavel %s sofrendo redefinicao.",//redefinition
			"semantico: operacao %s espera %s mas recebeu %s.",//op_wrong_type
			"semantico: divisao por zero.",//div_zero
			"semantico: tentativa de indexar variavel %s (um valor %s).",//attempt_index
			"semantico: indice de tipo %s.",//index_wrong_type
			"semantico: arranjo espera valor inteiro, booleano ou desconhecido mas recebeu %s.",//arr_type_not_allowed
			"semantico: operacao %s espera tipos iguais mas recebeu %s e %s.",//different_types
			"semantico: tentativa de chamar variavel %s (um valor %s).",//attempt_call
			"semantico: funcao espera retornar inteiro, booleano ou desconhecido mas retornou %s.",//func_type_not_allowed
	};

	enum ErrorType{ syntax_error, unknown_symbol, without_declaration,
					redefinition, op_wrong_type, div_zero, attempt_index, index_wrong_type,
					arr_type_not_allowed, different_types, attempt_call, func_type_not_allowed };

	void throwErr(ErrorType error, ...);
}
