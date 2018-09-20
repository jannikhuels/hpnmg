#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_monte.h>
#include <gsl/gsl_monte_plain.h>
#include <gsl/gsl_monte_miser.h>
#include <gsl/gsl_monte_vegas.h>
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

    	static double functionToIntegrateGauss(double x, void* data);

		static double transformedFunctionToIntegrateMonteCarlo(double *k, size_t dim, void* params);

       	static double getDensity(pair<string, map<string, float>> distribution, double value);

		double calculateIntervalsGauss(const ParametricLocation &location, ParametricLocationTree &tree, double timepoint, int nodeID, int evaluations);

        double calculateIntervalsMonteCarlo(const ParametricLocation &location, ParametricLocationTree &tree, double timepoint, int nodeID, char algorithm, int functioncalls, double &error);

        double getProbability(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, char algorithm, int functioncalls, int evaluations, double &totalerror);


    public:

    	ProbabilityCalculator();

        double getProbabilityGauss(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int evaluations);

        double getProbabilityMonteCarloPlain(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);

        double getProbabilityMonteCarloMiser(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);

        double getProbabilityMonteCarloVegas(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);


    };
}
