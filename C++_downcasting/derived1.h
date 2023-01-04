#ifndef DERIVED1_H
#define DERIVED1_H

#include "base.h"

class Derived1 : public Base {
  public:
    Derived1() { };
    ~Derived1() { }; 
    void print(); //notice that print does not exist in Base.
};

#endif