#include "../knights_tour_zdd.cpp"

#include "adapter.h"

// ========================================================================== //
int main(int argc, char** argv)
{
  run_knights_tour<cudd_zdd_adapter>(argc, argv);
}