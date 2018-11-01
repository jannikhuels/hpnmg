# Hybrid Petri nets with general transitions

## Install

### Ubuntu 18.04

Almost all dependencies can be installed via `apt`. Only [CArL](https://smtrat.github.io/carl/) and
[Hypro](https://hypro.github.io/hypro/html/) need to be compiled manually. However, they register themselves with CMake
upon compilation so that no steps other than compiling them need to be taken.

##### A note on CArL and HyPro releases
As of October 2018, HyPro's latest release is 17.10 whereas CArl's is 18.10. Since the former compiles with C++14 and
the latter with C++17, you need to pick an older CArl release. For example, 17.10 works just fine.

#### Dependencies
1. These are needed for both CArl and HyPro
   ```
   # gmpxx the Gnu Multiprecision library C++ interface
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
Download a [release](https://github.com/smtrat/carl/releases), for example 17.10 and prepare to build it.
```
$ cd the-extracted-release-folder
$ mkdir build && cd build && cmake ..
```
If your dependencies are installed correctly, the output of the latter
command should include lines similar to these:
```
-- Use system version of GMP/GMPXX 6.1.2
-- Use system version of Boost 106501
-- Use system version of Eigen3 3.3.4

```
Build carl with `make lib_carl`. This may take a minute.

#### HyPro
Download a [release](https://github.com/hypro/hypro/releases), for example 17.10 and prepare to build it.
```
$ cd the-extracted-release-folder
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