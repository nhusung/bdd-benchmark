#include "../game-of-life.cpp"

#include "adapter.h"

int
main(int argc, char** argv)
{
  return run_gameoflife<sylvan_bcdd_adapter>(argc, argv);
}
