#include "../apply.cpp"

#include "adapter.h"

int
main(int argc, char** argv)
{
  return run_apply<cudd_zdd_adapter>(argc, argv);
}
