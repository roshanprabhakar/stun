#include <stdio.h>
#include <vector>
#include <memory>

#include "poly.hpp"

int main()
{
  std::vector<std::unique_ptr<base>> elems;

  elems.push_back(std::unique_ptr<base>(new base));
  elems.push_back(std::unique_ptr<base>(new child));
  elems.push_back(std::unique_ptr<base>(new obj1));

  for (int i = 0; i < elems.size(); ++i)
    {
      elems[i]->dump();
    }

  return 0;
}
