#include "sylvan_init.cpp"

#include "common.cpp"
#include "pigeonhole_principle.cpp"

using namespace sylvan;

// =============================================================================
int main(int argc, char** argv)
{
  size_t N = 8;
  size_t M = 128;
  parse_input(argc, argv, N, M);

  size_t largest_bdd = 0;

  // =========================================================================
  INFO("Pigeonhole Principle for %zu : %zu (Sylvan %zu MB):\n", N+1, N, M);
  SYLVAN_INIT(M)

  // =========================================================================
  BDD sat_acc = sylvan_true;
  sylvan_protect(&sat_acc);

  const auto sat_and_clause = [&](clause_t &clause) -> void
  {
    BDD c = sylvan_false;
    sylvan_protect(&c);

    for (auto it = clause.rbegin(); it != clause.rend(); it++)
    {
      literal_t v = *it;
      c = sylvan_makenode(v.first,
                          v.second ? sylvan_true : c,
                          v.second ? c : sylvan_true);
    }

    sat_acc = sylvan_and(sat_acc, c);
    sylvan_unprotect(&c);

    largest_bdd = std::max(largest_bdd, sylvan_nodecount(c));
    largest_bdd = std::max(largest_bdd, sylvan_nodecount(sat_acc));
  };

  const auto sat_quantify_variable = [&](uint64_t var) -> void
  {
    sat_acc = sylvan_exists(sat_acc, sylvan_ithvar(var));
  };

  const auto sat_is_false = [&]() -> bool
  {
    return sat_acc == sylvan_false;
  };

  // =========================================================================
  INFO(" | CNF:\n");

  auto t1 = get_timestamp();

  sat_solver solver;
  construct_PHP_cnf(solver, N);

  auto t2 = get_timestamp();

  INFO(" | | variables:         %zu\n", label_of_Pij(N+1, N, N));
  INFO(" | | clauses:           %zu\n", solver.cnf_size());
  INFO(" | | time (ms):         %zu\n", duration_of(t1,t2));

  // =========================================================================
  INFO(" | BDD Solving:\n");

  auto t3 = get_timestamp();
  bool satisfiable = solver.is_satisfiable(sat_and_clause,
                                           sat_quantify_variable,
                                           sat_is_false);
  auto t4 = get_timestamp();

  INFO(" | | largest size:      %i\n", largest_bdd);
  INFO(" | | final size:        %i\n", sylvan_nodecount(sat_acc));
  INFO(" | | time (ms):         %zu\n", duration_of(t3,t4));


  // =========================================================================
  INFO(" | solution:            %s\n", satisfiable ? "SATISFIABLE" : "UNSATISFIABLE");
  SYLVAN_DEINIT

  exit(satisfiable ? -1 : 0);
}
