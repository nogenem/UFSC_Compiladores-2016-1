#pragma once

#include <string>
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

const std::vector<std::string> opName = {
  "soma", "subtracao", "multiplicacao", "divisao",
  "atribuicao", "igual", "diferente", "maior",
  "maior ou igual", "menor", "menor ou igual", "e",
  "ou", "nao", "menos unario", "parenteses"
};

}

namespace Types {

enum Type {unknown_t, integer_t, real_t, bool_t};

const std::vector<std::string> mascType = {
  "desconhecido",
  "inteiro",
  "real",
  "booleano"
};

const std::vector<std::string> femType = {
  "desconhecida",
  "inteira",
  "real",
  "booleana"
};

}
