#ifndef DERIVED2_H
#define DERIVED2_H

#include "base.h"

class Derived2 : public Base {
  public:
    Derived2() { };
    ~Derived2() { };
    void print(); //notice that print does not exist in Base.
};

#endif