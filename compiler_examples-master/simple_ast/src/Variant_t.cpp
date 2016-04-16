#include "Variant_t.h"
#include <iostream>

using namespace VAR;

void Variant_t::setType(Type t){
  _type = t;
  switch (t) {
    case integer_t: _int_v = 0; break;
    case double_t: _double_v = 0.0; break;
  }
}

void Variant_t::setValue(double v){
  switch (_type) {
    case integer_t: _int_v = (int)v; break;
    case double_t: _double_v = v; break;
  }
}

Variant_t Variant_t::operator+ (Variant_t& v){
  double v1 = (_type == integer_t) ? _int_v : _double_v;
  double v2 = (v.getType() == integer_t) ? v.getIntValue() : v.getDoubleValue();

  double r = v1 + v2;
  return (_type==integer_t && v.getType()==integer_t) ? Variant_t((int)r) : Variant_t(r);
}

Variant_t Variant_t::operator* (Variant_t& v){
  double v1 = (_type == integer_t) ? _int_v : _double_v;
  double v2 = (v.getType() == integer_t) ? v.getIntValue() : v.getDoubleValue();

  double r = v1 * v2;
  return (_type==integer_t && v.getType()==integer_t) ? Variant_t((int)r) : Variant_t(r);
}
