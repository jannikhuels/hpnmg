#pragma once

//#include "ParametricLocationTree.h"
#include <math.h>
#include <iostream>
extern "C"{
#include "integration/gauss_legendre.h"
}

#include <boost/math/distributions/normal.hpp> // for normal_distribution
  using boost::math::normal; // typedef provides default type is double.

#include <iostream>
  using std::cout; using std::endl; using std::left; using std::showpoint; using std::noshowpoint;
#include <iomanip>
  using std::setw; using std::setprecision;
#include <limits>
  using std::numeric_limits;


namespace hpnmg {

    class ProbabilityCalculator {


    private:

    	static double normalPdf(double x, void* data);

    	static double f(double x, void* data);

    	static const int n = 8; // 128; //1024;

    public:

    	ProbabilityCalculator();

        double getProbability();

    };
}
