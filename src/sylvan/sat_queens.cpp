#include "../sat_queens.cpp"

#include "adapter.h"

int main(int argc, char** argv)
{
  run_sat_queens<sylvan_bdd_adapter>(argc, argv);
}