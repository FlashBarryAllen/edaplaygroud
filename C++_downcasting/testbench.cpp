#include <iostream>
#include "derived1.h"
#include "derived2.h"

int main() {
  std::cout << "Welcome to Downcasting Example." << std::endl << std::endl;
  Base* ptr1 = new Derived1();
  Base* ptr2 = new Derived2();
 
  // now we can try a dynamic downcast:
  Derived1* ptr3 = dynamic_cast<Derived1*>(ptr1);
  // if successful, ptr3 will be non-NULL:
  if(ptr3)
    ptr3->print();
  else
    std::cout << "Could not downcast ptr1 to Derived1 class!" << std::endl;
 
  // if we try with ptr2 it will not work because ptr2 is pointing
  // to an object of class Derived2.
  Derived1* ptr4 = dynamic_cast<Derived1*>(ptr2);
  // if successful, ptr4 will be non-NULL:
  if(ptr4)
    ptr4->print();
  else
    std::cout << "Could not downcast ptr2 to Derived1 class!" << std::endl;
 
  delete ptr1;
  delete ptr2;
  // DO NOT DELETE ptr3 or ptr4 because they point to the same objects
  // as ptr1 and ptr2, i.e., casting does not copy the pointed-to objects.
  std::cout << std::endl << "Downcasting Example complete." << std::endl;
  return 0;
};