#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Region.h"

#include "Eigen/Geometry"

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

        /**
         * Controls whether or not to apply the integration bounds transformation provided by
         * <code>allDims::transformationMatrix</code>.
         */
        bool applyTransformation = false;
        /**
         * An augmented matrix representing an affine transformation applied to the integrator according to the
         * transformation theorem.
         *
         * This to calculate the integral over a different (transformed) set of intetragion bounds.
         *
         * <code>allDims::applyTransformation</code> must be true for this to take effect.
         */
        Eigen::MatrixXd transformationMatrix;

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

		/**
		 * Note that the <code>distributions</code> is slightly different from the ones used in the
		 * <code>getProbabilityForLocation...</code> functions.
*
		 * @param region
		 * @param distributionsNormalized The probability distribution for each dimension of <code>region</code>.
		 * @param algorithm
		 * @param functioncalls
		 * @param error
		 * @return
		 */
		double getProbabilityForRegionUsingMonteCarlo(const Region &region, const vector<pair<string, map<string, float>>> &distributionsNormalized, char algorithm, int functioncalls, double &error);


        double getProbabilityForIntersectionOfRegionsUsingMonteCarlo(const vector<Region> &regions, const vector<pair<string, map<string, float>>> &distributionsNormalized, char algorithm, int functioncalls, double &error);

        double getProbabilityForUnionOfRegionsUsingMonteCarlo(const vector<Region> &regions, const vector<pair<string, map<string, float>>> &distributionsNormalized, char algorithm, int functioncalls, double &error);


		double computeMultivariateIntegralUsingGauss(int evaluations, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus);

		double computeMultivariateIntegralUsingMonteCarlo(int functioncalls, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus, char algorithm, double &error);


    };
}
