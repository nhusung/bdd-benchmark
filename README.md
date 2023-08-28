# BDD Benchmark
This repository contains multiple examples for use of BDDs to solve various
problems. These are all implemented in exactly the same way, thereby allowing
one to compare implementations.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [BDD Benchmark](#bdd-benchmark)
    - [Implementations](#implementations)
        - [Enforcing comparability](#enforcing-comparability)
    - [Dependencies](#dependencies)
    - [Usage](#usage)
    - [Combinatorial Benchmarks](#combinatorial-benchmarks)
        - [Knight's Tour [ZDD]](#knights-tour-zdd)
        - [Queens [BDD, ZDD]](#queens-bdd-zdd)
        - [Tic-Tac-Toe [BDD, ZDD]](#tic-tac-toe-bdd-zdd)
    - [Verification](#verification)
        - [Picotrav [BDD, ZDD]](#picotrav-bdd-zdd)
        - [QBF Solver [BDD]](#qbf-solver-bdd)
    - [License](#license)
    - [Citation](#citation)
    - [References](#references)

<!-- markdown-toc end -->

## Implementations
We provide all the benchmarks described further below for the following
libraries.

- [**Adiar**](https://github.com/ssoelvsten/adiar):
  An I/O Efficient implementation with iterative algorithms using priority
  queues to exploit a special sorting of BDD nodes streamed from and to the
  disk. These algorithms have no need for memoization or garbage collection,
  but, on the other hand, nodes are also not shareable between BDDs.


- [**BuDDy**](http://vlsicad.eecs.umich.edu/BK/Slots/cache/www.itu.dk/research/buddy/):
  An easy-to-use yet extensive implementation with depth-first algorithms using
  a unique node table and memoization. It also supports variable reordering.

  We use the version from [here](https://github.com/SSoelvsten/BuDDy) that is set
  up for building with CMake.


- [**CAL**](https://github.com/SSoelvsten/cal):
  A breadth-first implementation that exploits a specific level-by-level
  locality of nodes on disk to improve performance when dealing with large BDDs.
  Unlike Adiar it also supports sharing of nodes between BDDs at the cost of
  memoization and garbage collection.


- **CUDD**:
  The most popular BDD package. It uses depth-first algorithms and a unique node
  table and memoization and also supports complement edges, Zero-suppressed
  Decision Diagrams, and variable reordering.

  We use version 3.0.0 as distributed on the unofficial mirror
  [here](https://github.com/SSoelvsten/cudd).


- [**Sylvan**](https://github.com/trolando/sylvan):
  A parallel (multi-core) implementation with depth-first algorithms using a
  unique node table and memoization. It also uses complement edges and supports
  Zero-suppressed Decision Diagrams.

  We will *not* make use of the multi-core aspect to make the results
  comparable.

We hope to extend the number of packages. See
[issue #12](https://github.com/SSoelvsten/bdd-benchmark/issues/12) for a list
of BDD packages we would like to have added to this set of benchmarks. Any
help to do so is very much appreciated.

### Enforcing comparability
For comparability, we will enforce all packages to follow the same settings.

- Only use a single core.

- Packages will initialise its unique node table to its full potential size and
  have its operation cache (memoization table) set to of 64:1.

- Dynamic variable reordering is disabled.


## Dependencies
Almost packages interface with CMake or GNU Autotools, which makes installation
very simple after having initialised all submodules using the following command.

```
git submodule update --init --recursive
```

This also requires *CMake* and a *C++* compiler of your choice. The *Picotrav*
benchmark requires GNU Bison and Flex, which can be installed with.

```bash
apt install bison flex
```

**Adiar**

Adiar also has dependencies on the *Boost Library*, which can be installed as follows
```bash
apt install libboost-all-dev
```

**CUDD**

The project has been built on Linux and tested on Ubuntu 18.04 and 20.04. On
*Windows Subsystem for Linux* or *Cygwin* the automake installation will fail
due to _\r\n_ line endings. To resolve this, first install dos2unix, then
convert the line endings for the relevant files as shown below in the CUDD
folder.

```bash
apt install dos2unix
find . -name \*.m4|xargs dos2unix
find . -name \*.ac|xargs dos2unix
find . -name \*.am|xargs dos2unix
```
Alternatively, run `make clean`.

Installation of CUDD seems neither possible without also building the
documentation. For this, you need a local installation of LaTeX.
```bash
sudo apt install texlive texlive-latex-extra
```

**Sylvan**

Sylvan also needs the *The GNU Multiple Precision Arithmetic* and *Portable
Hardware Locality* libraries, which can be installed as follows
```bash
apt install libgmp-dev libhwloc-dev
```


## Usage

All interactions have been made easy by use of the *makefile* at the root.

| Target  | Description                               |
|---------|-------------------------------------------|
| `build` | Build all dependencies and all benchmarks |
| `clean` | Remove all build artifacts                |

To build all BDD packages with *statistics*, set the Make variable *STATS* to
*ON* (default *OFF*).

Each benchmark below also has its own *make* target too for ease of use. You may
specify as a make variable the instance size *N* to solve, the amount of
_M_emory (MiB) to use in, and the *V*ariant (i.e. BDD package). For example, to
solve the combinatorial Queens with CUDD for *N* = 10 and with 256 MiB of memory
you run the following target.

Note, that the memory you set is only for the BDD package. So, the program will
either use swap or be killed if the BDD package takes up more memory in
conjunction with the benchmark's auxiliary data structures.

```bash
make combinatorial/queens V=cudd N=10 M=256
```

We provide benchmarks for both *Binary* and for *Zero-suppressed* Decision
Diagrams. Not all benchmarks support both types of decision diagrams, but if
possible, then the type can be chosen as part of the make target. We will below
mark whether a benchmark supports **BDD**s or **ZDD**s.

```bash
make combinatorial/queens/zdd V=cudd N=10 M=256
```

Some benchmarks allow for choosing between a set of **O**ptions, e.g. variable
ordering or algorithm.

```bash
make verification/picotrav V=cudd O=LEVEL_DFS
```

## Combinatorial Benchmarks

### Knight's Tour [ZDD]
Solves the following problem:

> Given N, then how many hamiltonian paths can a single Knight do across a chess
> board of size (N/2)x(N-N/2)?

Optionally, you can pick the type of tours to be computed:

- `open`: All hamiltonian path, i.e. one does not need to end back at the start.
- `closed`: Only hamiltonian cycles, i.e. where one does end back where one
  started.

The ZDD encoding is based on [[Bryant2021](#references)]. We represent all
O(N<sup>4</sup>) states, i.e. position and time, as a separate variable; a
transition relation then encodes the legal moves between two time steps. By
intersecting moves at all time steps we obtain all paths. On-top of this,
hamiltonian constraints are added and finally the size of the set of Knight's
Tours is obtained.

**Statistics:**

| Variable                | Value         |
|-------------------------|---------------|
| Labels                  | N<sup>4</sup> |
| Intersection operations | 2N<sup>4</sup> |

### Queens [BDD, ZDD]
Solves the following problem:

> Given N, then in how many ways can N queens be placed on an N x N chess board
> without threatening eachother?

Our implementation of these benchmarks are based on the description of
[[Kunkle10](#references)]. We construct an BDD row-by-row that represents
whether the row is in a legal state: is at least one queen placed on each row
and is it also in no conflicts with any other? On the accumulated BDD we then
count the number of satisfying assignments.

**Statistics:**

| Variable         | Value  |
|------------------|--------|
| Labels           | N²     |
| Apply operations | N² + N |


### Tic-Tac-Toe [BDD, ZDD]
Solves the following problem:

> Given N, then in how many ways can Player 1 place N crosses in a 3D 4x4x4 cube
> and have a tie, when Player 2 places noughts in all remaining positions?

This benchmark stems from [[Kunkle10](#references)]. Here we keep an accumulated
BDD on which we add one of the 76 constraints of at least one cross and one
nought after the other. We add these constraints in a different order than
[[Kunkle10](#references)], which does result in an up to 100 times smaller largest
intermediate result.

**Statistics:**

| Variable          |           Value |
|-------------------|-----------------|
| Labels            |              64 |
| Apply operations  |              76 |
| Initial BDD size  | (64-N+1)(N+1)-1 |

## Verification

### Picotrav [BDD, ZDD]
This benchmark is a small recreation of the *Nanotrav* example provided with the
CUDD library [[Somenzi2015](#references)]. Given a hierarchical circuit in (a
subset of the) [Berkeley Logic Interchange Format
(BLIF)](https://course.ece.cmu.edu/~ee760/760docs/blif.pdf) a BDD is created for
every net; dereferencing BDDs for intermediate nets after they have been used
for the last time. If two *.blif* files are given, then BDDs for both are
constructed and every output net is compared for equality.

BDD variables represent the value of inputs. Hence, a good variable ordering is
important for a high performance. To this end, one can use various variable
orderings derived from the given net.

- `input`: Use the order in which they are declared in the input *.blif* file.

- `dfs`: Variables are ordered based on a DFS traversal where non-input gates
  are recursed to first; thereby favouring deeper nodes.

- `level`: Variables are ordered based on the deepest reference by another net.
  Ties are broken based on the declaration order in the input (`input`).

- `level_dfs`: Similar to `level` but ties are broken based on the ordering in
  `dfs` rather than `input`.

- `random`: A randomized ordering of variables.

The *.blif* file(s) is given with the `-f` parameter (*F1* and *F2* Make
variables) and the variable order with `-o` (*O* for Make). You can find
multiple inputs in the *benchmarks/* folder.

```bash
make verification/picotrav F1=benchmarks/not_a.blif F2=benchmarks/not_b.blif V=cudd O=LEVEL_DFS
```

### QBF Solver [BDD]
Based on Jaco van de Pol's Christmas holiday hobbyproject, this is an
implementation of a QBF solver. Given an input in the
[*qcir*](https://www.qbflib.org/qcir.pdf) format, the decision diagram
representing each gate is computed bottom-up. The outermost quantifier in the
*prenex* is not resolved with BDD operations. Instead, if the decision diagram
has not already collapsed to a terminal, a witness/counter-example is obtained
from the diagram.

Each BDD variable corresponds to an input variables, i.e. a literal, of the
*qcir* circuit. To keep the decision diagrams small, one can choose from using
different variable orders; each of these are derived from the given circuit
during initialisation.

- `input`: Use the order in which variables are introduced in the *.qcir* file.

- `df`/`depth-first`: Order variables based on their first occurence in a
  depth-first traversal of the circuit.

- `df_rtl`/`depth-first_rtl`: Same as `df` but the depth-first traversal is
  *right-to-left* for each gate in the circuit.

- `level`/`level_df`: Order variables first based on their deepest reference by
  another gate. Ties are broken based on the depth-first (`df`) order.

If the `input` ordering is used, then the gates of the circuit are also resolved
in the order they were declared. Otherwise for `df` and `level`, gates are
resolved in a bottom-up order based on their depth within the circuit.

The *.qcir* file is given with the `-f` parameter (*F* Make variable) and the
ordering with `-o` (*O* for Make). Some small inputs can be found in the
*benchmarks/* folder.

```bash
make verification/qbf F=benchmarks/qcir/example_a.blif V=cudd O=LEVEL
```

## License
The software files in this repository are provided under the
[MIT License](/LICENSE.md).

## Citation
If you use this repository in your work, we sadly do not yet have written a
paper on this repository alone (this will be done though). In the meantime,
please cite the initial paper on *Adiar*.
```bibtex
@InProceedings{soelvsten2022:TACAS,
  title         = {Adiar: Binary Decision Diagrams in External Memory},
  author        = {S{\o}lvsten, Steffan Christ
               and van de Pol, Jaco
               and Jakobsen, Anna Blume
               and Thomasen, Mathias Weller Berg},
  year          = {2022},
  booktitle     = {Tools and Algorithms for the Construction and Analysis of Systems},
  editor        = {Fisman, Dana
               and Rosu, Grigore},
  pages         = {295--313},
  numPages      = {19},
  publisher     = {Springer},
  series        = {Lecture Notes in Computer Science},
  volume        = {13244},
  isbn          = {978-3-030-99527-0},
  doi           = {10.1007/978-3-030-99527-0\_16},
}
```

## References

- [[Bryant2021](https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py)]
  Bryant, Randal E. “*hpath.py*”. In: *Cloud-BDD* (GitHub). 2021

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “*Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams*”. In: *PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation*. 2010

- [[Bryant2021](https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py)]
  Bryant, Randal E. “*hpath.py*”. In: *Cloud-BDD* (GitHub). 2021

- [[Somenzi2015](https://github.com/ssoelvsten/cudd)]
  Somenzi, Fabio: *CUDD: CU decision diagram package, 3.0*. University
  of Colorado at Boulder. 2015
