#pragma once

#include <vector>

namespace Ops {

// Possiveis operações, tanto binarias quanto unárias
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

// Possiveis tipos de variaveis, arranjos e afins
enum Type { unknown_t, integer_t, real_t, bool_t, composite_t };

const std::vector<const char*> mascType = {
  "desconhecido",
  "inteiro",
  "real",
  "booleano",
  "composto"
};
const std::vector<const char*> femType = {
  "desconhecida",
  "inteira",
  "real",
  "booleana",
  "composta"
};

}

namespace Kinds {

// Possiveis tipos que um Nodo variavel pode ser
enum Kind{ unknown_t, variable_t, array_t, function_t, type_t };

const std::vector<const char*> kindName = {
  "desconhecido",
  "variavel",
  "arranjo",
  "funcao",
  "tipo"
};

}

namespace Errors {

// Mensagens de erros que podem acontecer
const std::vector<const char*> messages = {
  "syntax error",//syntax_error
  "lexico: simbolo desconhecido: %c.",//unknown_symbol
  "semantico: %s %s sem declaracao.",//without_declaration
  "semantico: %s %s nao inicializada.",//not_initialized
  "semantico: %s %s sofrendo redefinicao.",//redefinition
  "semantico: %s %s com uso como %s.",//wrong_use
  "semantico: operacao %s espera %s mas recebeu %s.",//op_wrong_type1
  "semantico: operacao %s espera inteiro ou real mas recebeu %s.",//op_wrong_type2
  "semantico: indice de tipo %s.",//index_wrong_type
  "semantico: arranjo %s com tamanho menor do que um.",//array_size_lst_1
  "semantico: funcao %s declarada mas nunca definida.",//func_never_defined
  "semantico: parametro espera %s mas recebeu %s.",//param_wrong_type
  "semantico: arranjo %s possui tamanho menor que o necessario.",//array_size_lst_needed
  "semantico: funcao %s espera %d parametros mas recebeu %d.",//func_wrong_param_amount
  "semantico: tipo %s nao contem componente %s.",//type_wrong_comp
  "semantico: funcao sem chamada de retorno.",//func_without_return
  "semantico: funcao %s sofrendo redeclaracao.",//func_redeclaration
  "semantico: uso de variavel ou valor como arranjo.",//param_value_as_array
  "semantico: tipo %s indefinido."//type_undefined
};

enum ErrorType {syntax_error, unknown_symbol, without_declaration, not_initialized,
  redefinition, wrong_use, op_wrong_type1, op_wrong_type2, index_wrong_type,
  array_size_lst_1, func_never_defined, param_wrong_type, array_size_lst_needed,
  func_wrong_param_amount, type_wrong_comp, func_without_return, func_redeclaration,
  param_value_as_array, type_undefined};

void print(ErrorType error, ...);

}
