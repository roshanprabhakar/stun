#include <stdio.h>
#include "poly.hpp"

void base::dump()
{
  printf("-----------\n");
  printf ("base impl\n");
}

third_party::third_party()
  : state(0)
{ }

void third_party::dump()
{
  printf("3rd party state: %d\n", state);
}

void child::dump()
{
  base::dump();
  third_party::dump();
  printf ("child impl\n");
}

obj1::obj1()
{ }
