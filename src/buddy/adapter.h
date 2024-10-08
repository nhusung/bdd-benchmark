#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

#include "../common/adapter.h"

#include <bdd.h>

////////////////////////////////////////////////////////////////////////////////
/// Initialisation of BuDDy. The size of each node in the unique table is 6*4 =
/// 24 bytes (BddNode in kernel.h) while each cache entry takes up 4*4 = 16
/// bytes (BddCacheData in cache.h).
///
/// So, the memory in bytes occupied when given NODE_SLOTS and CACHE_SLOTS is
///
///                       24 * NODE_SLOTS + 16 * CACHE_SLOTS
///
/// - bdd_init:
///     We initialise BuDDy with a unique table of some number of nodes and a
///     cache with a set number of entries. The nodetable may grow, if need be
///     (except if something else is specified).
///
///     The initial size of the nodetable is in fact not the given table size,
///     but rather the smallest prime number larger than the given value.
///
/// - bdd_setmaxincrease:
///     The amount the original unique table is allowed to be increased during
///     garbage collection. If it is set to 0, then you fix the current size.
///
/// - bdd_setmaxnodesize
///     Sets the maximum number of nodes in the nodetable.
///
/// - bdd_setcacheratio:
///     Allows the cache to grow in size together with the nodetable. This
///     specifies the ratio between the node table and the cache. If it is not
///     called, then the cache is of a fixed size.
///
/// - bdd_setvarnum:
///     Declare the number of variables to expect to be used.
////////////////////////////////////////////////////////////////////////////////

/// Largest number in BuDDy
constexpr size_t max_int = std::numeric_limits<int>::max();

/// Number of table entries per cache entry (double of what is recommended by BuDDy).
constexpr size_t min_cache_ratio = 32;

/// Number of table entries per cache entry (as recommended by BuDDy).
constexpr size_t max_cache_ratio = 64;

/// Size of a BDD node in BuDDy
constexpr size_t sizeof_node = 24u;

/// Size of a Cache entry in BuDDy
constexpr size_t sizeof_cache = 16u;

/// Number of Caches in BuDDy
constexpr size_t caches = 6u;

inline int
table_size(const size_t memory_bytes)
{
  // Number of bytes to be used for a single set of table and cache entries.
  constexpr size_t sizeof_norm = sizeof_node * max_cache_ratio + sizeof_cache * caches;

  // Compute number of nodes possible
  const size_t nodes = (memory_bytes / sizeof_norm) * max_cache_ratio;
  assert(nodes * sizeof_node + (nodes / max_cache_ratio) * sizeof_cache <= memory_bytes);

  // Cap at the maximum possible size
  return std::min<size_t>(nodes, max_int);
}

inline int
cache_size(const size_t memory_bytes, const int nodes)
{
  // Cache size according to largest ratio
  const size_t min_cache = nodes / max_cache_ratio;

  // Cache size according to smallest ratio
  const size_t max_cache = nodes / min_cache_ratio;

  // Cache size according to remaining memory
  const size_t cache_memory = (memory_bytes - nodes * sizeof_node) / (sizeof_cache * caches);

  // Choose cache size based on remaining memory, but bounded from either side.
  return std::max(min_cache, std::min(max_cache, max_cache));
}

class buddy_bdd_adapter
{
public:
  static constexpr std::string_view name = "BuDDy";
  static constexpr std::string_view dd   = "BDD";

  static constexpr bool needs_extend     = false;
  static constexpr bool needs_frame_rule = true;

  static constexpr bool complement_edges = false;

public:
  typedef bdd dd_t;
  typedef bdd build_node_t;

private:
  const int _varcount;
  bdd _latest_build;

  bdd _vars_relnext;
  bddPair* _pairs_relnext = nullptr;

  bdd _vars_relprev;
  bddPair* _pairs_relprev = nullptr;

  // Init and Deinit
public:
  buddy_bdd_adapter(int varcount)
    : _varcount(varcount)
  {
    const size_t memory_bytes = static_cast<size_t>(M) * 1024 * 1024;

    const int nodes  = table_size(memory_bytes);
    const int caches = cache_size(memory_bytes, nodes);

    bdd_init(nodes, caches);

    // Set cache ratio if table changes in size. This is disabled, since the
    // table size is fixed below.
    // bdd_setcacheratio(cache_ratio);

    // Fix table to current initial size. BuDDy chooses a nodetable size the
    // closest prime BIGGER than the given number. This means, we cannot fix the
    // size with 'bdd_setmaxnodenum'. So, we must instead set it to never
    // increase.
    //
    // TODO: Find the largest prime smaller than the computed number of nodes?
    bdd_setmaxincrease(0);

    bdd_setvarnum(varcount);

    // Disable default gbc_handler
    bdd_gbc_hook(NULL);

    // Disable dynamic variable reordering
    if (!enable_reordering) { bdd_disable_reorder(); }

    this->_latest_build = bot();
    this->_vars_relnext = bot();
    this->_vars_relprev = bot();
  }

  ~buddy_bdd_adapter()
  {
    bdd_freepair(this->_pairs_relnext);
    bdd_freepair(this->_pairs_relprev);

    bdd_done();
  }

public:
  template <typename F>
  int
  run(const F& f)
  {
    return f();
  }

  // BDD Operations
public:
  inline bdd
  top()
  {
    return bddtrue;
  }

  inline bdd
  bot()
  {
    return bddfalse;
  }

  inline bdd
  ithvar(int i)
  {
    return bdd_ithvar(i);
  }

  inline bdd
  nithvar(int i)
  {
    return bdd_nithvar(i);
  }

  template <typename IT>
  inline bdd
  cube(IT rbegin, IT rend)
  {
    bdd res = top();
    while (rbegin != rend) { res = ite(ithvar(*(rbegin++)), res, bot()); }
    return res;
  }

  inline bdd
  cube(const std::function<bool(int)>& pred)
  {
    bdd res = top();
    for (int i = _varcount - 1; 0 <= i; --i) {
      if (pred(i)) { res = ite(ithvar(i), res, bot()); }
    }
    return res;
  }

  inline bdd
  apply_and(const bdd& f, const bdd& g)
  {
    return bdd_and(f, g);
  }

  inline bdd
  apply_diff(const bdd& f, const bdd& g)
  {
    return bdd_and(f, ~g);
  }

  inline bdd
  apply_imp(const bdd& f, const bdd& g)
  {
    return bdd_imp(f, g);
  }

  inline bdd
  apply_or(const bdd& f, const bdd& g)
  {
    return bdd_or(f, g);
  }

  inline bdd
  apply_xnor(const bdd& f, const bdd& g)
  {
    return bdd_biimp(f, g);
  }

  inline bdd
  apply_xor(const bdd& f, const bdd& g)
  {
    return bdd_xor(f, g);
  }

  inline bdd
  ite(const bdd& f, const bdd& g, const bdd& h)
  {
    return bdd_ite(f, g, h);
  }

  template <typename IT>
  inline bdd
  extend(const bdd& f, IT /*begin*/, IT /*end*/)
  {
    return f;
  }

  inline bdd
  exists(const bdd& f, int i)
  {
    return bdd_exist(f, bdd_ithvar(i));
  }

  inline bdd
  exists(const bdd& f, const std::function<bool(int)>& pred)
  {
    return bdd_exist(f, cube(pred));
  }

  template <typename IT>
  inline bdd
  exists(const bdd& f, IT rbegin, IT rend)
  {
    return bdd_exist(f, cube(rbegin, rend));
  }

  inline bdd
  forall(const bdd& f, int i)
  {
    return bdd_forall(f, bdd_ithvar(i));
  }

  inline bdd
  forall(const bdd& f, const std::function<bool(int)>& pred)
  {
    return bdd_forall(f, cube(pred));
  }

  template <typename IT>
  inline bdd
  forall(const bdd& f, IT rbegin, IT rend)
  {
    return bdd_forall(f, cube(rbegin, rend));
  }

  inline bdd
  relnext(const bdd& states, const bdd& rel, const bdd& /*rel_support*/)
  {
    if (!_pairs_relnext) {
      assert(_vars_relnext == bot());

      _vars_relnext = cube([](int x) { return x % 2 == 0; });

      _pairs_relnext = bdd_newpair();
      for (int i = _varcount - 2; 0 <= i; i -= 2) { bdd_setpair(_pairs_relnext, i + 1, i); }
    }

    return bdd_replace(bdd_appex(states, rel, bddop_and, _vars_relnext), _pairs_relnext);
  }

  inline bdd
  relprev(const bdd& states, const bdd& rel, const bdd& /*rel_support*/)
  {
    if (!_pairs_relprev) {
      assert(_vars_relprev == bot());

      _vars_relprev = cube([](int x) { return x % 2 == 1; });

      _pairs_relprev = bdd_newpair();
      for (int i = _varcount - 2; 0 <= i; i -= 2) { bdd_setpair(_pairs_relprev, i, i + 1); }
    }

    return bdd_appex(bdd_replace(states, _pairs_relprev), rel, bddop_and, _vars_relprev);
  }

  inline uint64_t
  nodecount(const bdd& f)
  {
    const uint64_t c = bdd_nodecount(f);
    // BuDDy does not count terminal nodes. If a BDD has no inner nodes, then it consists of a
    // single terminal node. Otherwise, both terminals are referenced.
    return c == 0 ? 1 : c + 2;
  }

  inline uint64_t
  satcount(const bdd& f)
  {
    return this->satcount(f, this->_varcount);
  }

  inline uint64_t
  satcount(const bdd& f, const size_t vc)
  {
    assert(vc <= this->_varcount);

    const double excess_variables = static_cast<double>(this->_varcount) - static_cast<double>(vc);
    return bdd_satcount(f) / std::pow(2, excess_variables);
  }

  inline bdd
  satone(const bdd& f)
  {
    return bdd_satone(f);
  }

  inline bdd
  satone(const bdd& f, const bdd& c)
  {
    return bdd_satoneset(f, c, bot());
  }

  inline std::vector<std::pair<int, char>>
  pickcube(const bdd& f)
  {
    std::vector<std::pair<int, char>> res;
    bdd sat = satone(f);

    while (sat != bddfalse && sat != bddtrue) {
      const int var      = bdd_var(sat);
      const bdd sat_low  = bdd_low(sat);
      const bdd sat_high = bdd_high(sat);

      const bool go_high = sat_high != bddfalse;
      res.push_back({ var, '0' + go_high });

      sat = go_high ? sat_high : sat_low;
    }
    return res;
  }

  void
  print_dot(const bdd& f, const std::string& filename)
  {
    FILE* p = fopen(filename.data(), "w");
    bdd_fprintdot(p, f);
    fclose(p);
  }

  void
  save(const bdd& f, const std::string& filename)
  {
    FILE* p = fopen(filename.data(), "w");
    bdd_save(p, f);
    fclose(p);
  }

  // BDD Build Operations
public:
  inline bdd
  build_node(const bool value)
  {
    const bdd res = value ? top() : bot();
    if (_latest_build == bot()) { _latest_build = res; }
    return res;
  }

  inline bdd
  build_node(const int label, const bdd& low, const bdd& high)
  {
    _latest_build = ite(bdd_ithvar(label), high, low);
    return _latest_build;
  }

  inline bdd
  build()
  {
    const bdd res = _latest_build;
    _latest_build = bot(); // <-- Reset and free builder reference
    return res;
  }

  // Statistics
public:
  inline size_t
  allocated_nodes()
  {
    return bdd_getnodenum();
  }

  void
  print_stats()
  {
    std::cout << "\nBuDDy statistics:\n";

    bddStat stats;
    bdd_stats(&stats);

    std::cout << "   Table:\n"
              << "   | total produced:      " << stats.produced
              << "\n"

              // Commented lines are only available if 'CACHESTATS' flag is set
              // bddCacheStat cache_stats;
              // bdd_cachestats(&cache_stats);

              // INFO(" | | access:              %zu\n", cache_stats.uniqueAccess);
              // INFO(" | | hits:                %zu\n", cache_stats.uniqueHit);
              // INFO(" | | miss:                %zu\n", cache_stats.uniqueMiss);
              // INFO(" | Cache:\n");
              // INFO(" | | hits:                %zu\n", cache_stats.opHit);
              // INFO(" | | miss:                %zu\n", cache_stats.opMiss);

              << "   Garbage Collections:   " << stats.gbcnum << "\n";
  }
};
