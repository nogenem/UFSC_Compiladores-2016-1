#pragma once

#include <vector>

/*
  Ideia vinda da versão 1.0 do professor.
*/
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

enum Type {unknown_t, integer_t, real_t, bool_t};

const std::vector<const char*> mascType = {
  "desconhecido",
  "inteiro",
  "real",
  "booleano"
};

const std::vector<const char*> femType = {
  "desconhecida",
  "inteira",
  "real",
  "booleana"
};

}

namespace Kinds {
  enum Kind{ unknown_t, variable_t, array_t, function_t };

  const std::vector<const char*> kindName = {
    "desconhecido",
    "variavel",
    "arranjo",
    "funcao"
  };
}

namespace Errors {

  const std::vector<const char*> messages = {
    "lexico: simbolo desconhecido: %c.",
    "semantico: %s %s sem declaracao.",//arranjo/variavel/funcao/tipo x
    "semantico: %s %s nao inicializada.",
    "semantico: %s %s sofrendo redefinicao.",
    "semantico: %s %s com uso como %s.",//variavel x arranjo/...
    "semantico: operacao %s espera %s mas recebeu %s.",
    "semantico: operacao %s espera inteiro ou real mas recebeu %s.",
    "semantico: indice de tipo %s.",
    "semantico: arranjo %s com tamanho menor que um.",
    "semantico: funcao %s declarada mas nunca definida.",
    "semantico: parametro espera %s mas recebeu %s.",
    "semantico: arranjo %s possui tamanho menor que o necessario.",
    "semantico: funcao %s espera %d parametros mas recebeu %d.",
    "semantico: funcao %s sem declaracao.",
    "semantico: tipo %s nao contem parametro %s."
  };

  enum ErrorType {unknown_symbol, without_declaration, not_initialized,
    redefinition, wrong_use, op_wrong_type1, op_wrong_type2, index_wrong_type,
    array_index_lst_1, func_never_declared, param_wrong_type, array_size_lst_needed,
    array_wrong_amount, func_without_declaration, type_wrong_param};

  void print(ErrorType error, ...);
}
