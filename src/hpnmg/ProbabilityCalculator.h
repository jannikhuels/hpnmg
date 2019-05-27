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


	typedef struct {
		double value;
		double transformedValue;
		pair<string, map<string, float>> distribution;
	} singleDim;



	typedef struct {
		int current_index;
		vector<singleDim> integrals;
		vector<vector<double>> lowerBounds;
		vector<vector<double>> upperBounds;
        int evaluations;
        double maxTime;
	} allDims;


    class ProbabilityCalculator {


    private:

    	static double functionToIntegrateGauss(double x, void* data);

		static double transformedFunctionToIntegrateMonteCarlo(double *k, size_t dim, void* params);

       	static double getDensity(pair<string, map<string, float>> distribution, double value);

        double getProbability(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, char algorithm, int functioncalls, int evaluations, double &totalerror);



    public:

    	ProbabilityCalculator();

        double getTotalProbabilityUsingGauss(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int evaluations);

        double getTotalProbabilityUsingMonteCarloPlain(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);

        double getTotalProbabilityUsingMonteCarloMiser(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);

		double getTotalProbabilityUsingMonteCarloVegas(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror);

		double getProbabilityForLocationUsingGauss(const ParametricLocation &location, vector<pair<string, map<string, float>>> distributions, double maxTime, int evaluations);

		double getProbabilityForLocationUsingMonteCarlo(const ParametricLocation &location, vector<pair<string, map<string, float>>> distributions, double maxTime, char algorithm, int functioncalls, double &error);

		double computeMultivariateIntegralUsingGauss(int evaluations, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus);

		double computeMultivariateIntegralUsingMonteCarlo(int functioncalls, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus, char algorithm, double &error);


    };
}
