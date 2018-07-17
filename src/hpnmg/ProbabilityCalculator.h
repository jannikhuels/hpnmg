#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include <math.h>
#include <iostream>
extern "C"{
#include "integration/gauss_legendre.h"
}

#include <boost/math/distributions.hpp> // for normal_distribution
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>


namespace hpnmg {

    class ProbabilityCalculator {


    private:

       	static const int n = 8;

    	static double functionToIntegrate(double x, void* data);

       	static double getDensity(pair<string, map<string, float>> distribution, double value);

		static double checkInfinity(double value);

		static double correctValue(double value);

//       	double calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint, map<int, vector<vector<double>>> &thresholds, bool &restricted, int nodeID, int parentID);
		double calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint, int nodeID);


    public:

    	ProbabilityCalculator();

        double getProbability(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint);


    };
}
