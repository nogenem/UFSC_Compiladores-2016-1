#include "util.hpp"
#include <iostream>

extern int yylineno;

using namespace Errors;

void Errors::print(ErrorType error, ...){
  va_list ap;
  va_start(ap, error);
  std::fprintf(stderr, "[Linha %d] Erro ", yylineno);
  std::vfprintf(stderr, messages[error], ap);
  std::fprintf(stderr, "\n");
}
