#include "../hamiltonian.cpp"

#include "adapter.h"

int
main(int argc, char** argv)
{
  return run_hamiltonian<oxidd_bcdd_adapter>(argc, argv);
}
