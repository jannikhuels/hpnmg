# Hybrid Petri nets with general transitions

## Install

### Ubuntu 18.04

Almost all dependencies can be installed via `apt`. Only [CArL](https://smtrat.github.io/carl/) and
[Hypro](https://hypro.github.io/hypro/html/) need to be compiled manually. However, they register themselves with CMake
upon compilation so that no steps other than compiling them need to be taken. 

##### A note on CArL and HyPro versions
Not all versions of CArl and HyPro work with each other or with HPnmG. Unfortunately, it is not always so clear which
*do*. Symptoms of bad versions include:
- HyPro refusing to compile because of C++17 features in CArl. CArl offers a C++14 branch `master14`, HyPro offers
  `c++14-support`.
- HPnmG or targets thereof refusing to compile because of changed include paths, classes or signatures in HyPro
- Some targets of HPnmG refusing to *link* because of undefined symbols from `libgmp` in `libhypro.so`

As of now (2019-07-22), you should be able to use [this revision](https://github.com/smtrat/carl/commit/ace90eb5daad)
for CArl and [this one](https://github.com/hypro/hypro/commit/9d26f57b5f62) for HyPro.

For some branches (`integrationOnPolytopes`, `nondeterministicRates`, `hybridAutomata`) you need the C++17 features of CArl. You should then use [this revision](https://github.com/smtrat/carl/commit/112ab9ffec0ce2252aec641c72905771c4729223) of CArl and [this one](https://github.com/hypro/hypro/commit/0754e84580dc0da450e809698f176a02e18bee25) for HyPro.

#### Dependencies
0. Install CMake.
   ```
   $ sudo apt install cmake
   ```
1. These are needed for both CArl and HyPro
   ```
   # gmp and gmpxx, the Gnu Multiprecision library and its C++ interface
   $ sudo apt install libgmp-dev

   # Boost
   $ sudo apt install libboost-all-dev
   ```
2. CArl additionally needs *Eigen3* for numerical computations
   ```
   $ sudo apt install libeigen3-dev
   ```
3. HyPro additionally needs these
   ```
   # CGAL
   $ sudo apt install libcgal-dev

   # GSL - GNU Scientific Library
   $ sudo apt install libgsl-dev

   # JRE
   $ sudo apt install default-jre

   # uuid-dev
   $ sudo apt install uuid-dev

   # Xerces-C++ XML Parser
   $ sudo apt install libxerces-c-dev
   ```

#### CArl
Download [CArl](https://github.com/smtrat/carl) and pick a [version](#a-note-on-carl-and-hypro-versions)
```
$ git clone https://github.com/smtrat/carl && cd carl && git checkout <XYZ>
$ mkdir build && cd build && cmake ..
```
If your dependencies are installed correctly, the output of the latter
command should include lines similar to these:
```
-- Use system version of GMP/GMPXX 6.1.2
-- Use system version of Boost 106501
-- Use system version of Eigen3 3.3.4

```
Build carl with `make carl`. This may take a minute.

#### HyPro
Download [HyPro](https://github.com/hypro/hypro) and pick a [version](#a-note-on-carl-and-hypro-versions)
```
$ git clone https://github.com/hypro/hypro && cd hypro && git checkout <XYZ>
$ mkdir build && cd build && cmake ..
```
If your dependencies are installed correctly and if CArl registered itself correctly with CMake, the output of the
latter command should include lines like these:
```
-- Use system version of CArL
-- Found Java: ...
```
Build HyPro's resources and HyPro itself with `make resources && make hypro`. This may take 10 minutes.

## Instructions

Replication of results for HSCC 2020 Submission "Classic and Non-Prophetic Model Checking for Hybrid Petri Nets with Stochastic Firings":

The GoogleTest file `test/testNondeterministicConflicts.cpp` holds tests for all versions of the case study used in the HSCC 2020 submission, for both nonprophetic and prophetic scheduling. The core algorithms presented in the submission can be found inside the file: `src/hpnmg/NondeterminismSolver.cpp`.
The model files can be found in the folder `/test/testfiles/nondeterministicModelsCharging/`. This folder also includes a PDF file `PLTsHSCC.pdf` that shows the Parametric Location Trees of the models presented in the submission.



