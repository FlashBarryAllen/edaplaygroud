#include <iostream>
#include "derived1.h"
#include "derived2.h"

void Derived1::print() {
  std::cout << "Downcast to Derived1 was successful!" << std::endl;
};
 
void Derived2::print() {
  std::cout << "Downcast to Derived2 was successful!" << std::endl;
};
