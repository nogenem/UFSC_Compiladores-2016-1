#pragma once

#include <stdint.h>

extern void yyerror(const char* s, ...);

namespace VAR {

  enum Type { integer_t, double_t };

  class Variant_t {
    public:
      Variant_t() : _int_v(0), _type(integer_t) {}
      Variant_t(int v) : _int_v(v), _type(integer_t) {}
      Variant_t(double v) : _double_v(v), _type(double_t) {}

      void setType(Type t);
      void setValue(double v);

      Type getType(){return _type;}
      int getIntValue(){return _int_v;}
      double getDoubleValue(){return _double_v;}

      Variant_t operator+ (Variant_t& v);
      Variant_t operator* (Variant_t& v);
    private:
      union{
        int _int_v;
        double _double_v;
      };
      Type _type;
  };

}
