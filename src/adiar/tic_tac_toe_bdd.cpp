#include "../tic_tac_toe_bdd.cpp"

#include "adapter.h"

int main(int argc, char** argv)
{
  return run_tic_tac_toe<adiar_bdd_adapter>(argc, argv);
}
