#include "../picotrav.cpp"

#include "adapter.h"

int
main(int argc, char** argv)
{
  return run_picotrav<oxidd_bdd_adapter>(argc, argv);
}
