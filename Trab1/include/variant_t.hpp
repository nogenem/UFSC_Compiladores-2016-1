#pragma once

extern void yyerror(const char* s, ...);

namespace VAR {

enum Type { integer_t, real_t, bool_t };

class Variant_t {
  public:
    Variant_t(){}
};

}
