#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include <math.h>
#include <iostream>
extern "C"{
#include "integration/gauss_legendre.h"
}

#include <boost/math/distributions/normal.hpp> // for normal_distribution
  using boost::math::normal; // typedef provides default type is double.

#include <iostream>
#include <iomanip>
#include <limits>


namespace hpnmg {

    class ProbabilityCalculator {


    private:

       	static const int n = 8; // 128; //1024;

   // 	static double normalPdf(double x, void* data);


    	static double f(double x, void* data);


       	double calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree);

    public:

    	ProbabilityCalculator();

        double getProbability(const vector<ParametricLocationTree::Node> &nodes, const ParametricLocationTree &tree);


    };
}
