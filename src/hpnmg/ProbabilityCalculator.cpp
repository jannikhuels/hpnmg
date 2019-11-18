#include "ProbabilityCalculator.h"

#include "Eigen/Geometry"
#include "Eigen/LU"

#include <cassert>
#include <cmath>

#include "helper/Triangulation.h"

using namespace std;

namespace hpnmg {


ProbabilityCalculator::ProbabilityCalculator(){}




    double ProbabilityCalculator::getTotalProbabilityUsingGauss(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int evaluations){

        double placeholder;
        return getProbability(nodes, tree,  timepoint, 0, 0, evaluations, placeholder);

    }



    double ProbabilityCalculator::getTotalProbabilityUsingMonteCarloPlain(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

        return getProbability(nodes, tree,  timepoint, 1, functioncalls, 0, totalerror);
    }



    double ProbabilityCalculator::getTotalProbabilityUsingMonteCarloMiser(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

        return getProbability(nodes, tree,  timepoint, 2, functioncalls, 0, totalerror);
    }



    double ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

        return getProbability(nodes, tree,  timepoint, 3, functioncalls, 0, totalerror);
    }



    double ProbabilityCalculator::getProbability(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, char algorithm, int functioncalls, int evaluations, double &totalerror){

        double total = 0.0;
        double error = 0.0;
        totalerror = 0.0;
        double nodeResult = 0.0;

        for (int i = 0;i < nodes.size(); i++) {
            cout << "----" << endl;
            cout << "Node: " << nodes[i].getNodeID() << endl;
            cout << "Normed: " << nodes[i].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed() << " - ";

            if (algorithm == 0)
                //Gauss Legendre
                nodeResult = getProbabilityForLocationUsingGauss(nodes[i].getParametricLocation(), tree.getDistributions(), tree.getMaxTime(), evaluations) * nodes[i].getParametricLocation().getAccumulatedProbability();
            else
                //Monte Carlo
                nodeResult = getProbabilityForLocationUsingMonteCarlo(nodes[i].getParametricLocation(), tree.getDistributions(), tree.getMaxTime(), algorithm, functioncalls, error) * nodes[i].getParametricLocation().getAccumulatedProbability();

            cout << "----" << endl;
            cout << "Node Result: p=" << nodeResult << " +- " << error << endl;

            total += nodeResult;
            totalerror += error;
        }

        cout << "----" << endl;
        cout << "Total: " << total << " +- " << totalerror << endl;
        cout << "----" << endl;
        cout << "----" << endl;

        return total;

    }



	double ProbabilityCalculator::getProbabilityForLocationUsingGauss(const ParametricLocation &location, vector<pair<string, map<string, float>>> distributions, double maxTime, int evaluations) {


        double completeResult = 0.0;
        double result;



        for (std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals : location.getIntegrationIntervals()) {

            allDims all;
            allDims allPlus;
            allDims allMinus;

            all.current_index = 0;
            allPlus.current_index = 0;
            allMinus.current_index = 0;

          //  cout << "Max time: " << maxTime << " - ";

            //std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = location.getIntegrationIntervals();
            vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
            int dimension = location.getDimension();

          //  cout << "GTF: " << generalTransitionsFired << " - ";
          //  cout << "Accumulated Probability: " << location.getAccumulatedProbability() << endl << endl;

            for (int i = 0; i < integrationIntervals.size(); i++) {
                singleDim sAll;
                sAll.distribution = distributions[integrationIntervals[i].first];
                //sAll.distribution = distributions[0];
                all.integrals.push_back(sAll);
                singleDim sPlus;
                sPlus.distribution = sAll.distribution;
                allPlus.integrals.push_back(sPlus);
                singleDim sMinus;
                sMinus.distribution = sAll.distribution;
                allMinus.integrals.push_back(sMinus);


                all.lowerBounds.push_back(integrationIntervals[i].second.first);
                all.upperBounds.push_back(integrationIntervals[i].second.second);
                allPlus.lowerBounds.push_back(integrationIntervals[i].second.first);
                allPlus.upperBounds.push_back(integrationIntervals[i].second.second);
                allMinus.lowerBounds.push_back(integrationIntervals[i].second.first);
                allMinus.upperBounds.push_back(integrationIntervals[i].second.second);


                if (isinf(integrationIntervals[i].second.second[0])) {

                       int last = allPlus.upperBounds.size() - 1;

                       fill(all.upperBounds[last].begin(), all.upperBounds[last].end(), 0.0);
                       all.upperBounds[last][0] = maxTime;


                       //Faking a distribution with probability 1
                       fill(allPlus.upperBounds[last].begin(), allPlus.upperBounds[last].end(), 0.0);
                       allPlus.upperBounds[last][0] = maxTime;
                       fill(allPlus.lowerBounds[last].begin(), allPlus.lowerBounds[last].end(), 0.0);
                       pair<string, map<string, float>> uniform;
                       uniform.first = "uniform";
                       std::map<string, float> params;
                       params["a"] = 0.0;
                       params["b"] = maxTime;
                       uniform.second = params;
                       allPlus.integrals[last].distribution = uniform;

                       fill(allMinus.upperBounds[last].begin(), allMinus.upperBounds[last].end(), 0.0);
                       allMinus.upperBounds[last][0] = maxTime;
                       fill(allMinus.lowerBounds[last].begin(), allMinus.lowerBounds[last].end(), 0.0);

                }

            }

            result = computeMultivariateIntegralUsingGauss(evaluations, all, allPlus, allMinus);

            completeResult += result;
            //cout << "Local Result " << result << endl;

        }

        return completeResult;

    }



    double ProbabilityCalculator::computeMultivariateIntegralUsingGauss(int evaluations, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus){

        double result = 0.0;

        double resultAll = 0.0;
        double resultPlus = 0.0;
        double resultMinus = 0.0;


		if (all.integrals.size() > 0){


		    all.evaluations = evaluations;
			double lower = all.lowerBounds[0][0];
			double upper = all.upperBounds[0][0];

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0  || lower >= upper)
                resultAll =  0.0;
            else
                resultAll = gauss_legendre(64, functionToIntegrateGauss, &all, lower, upper);



            allPlus.evaluations = evaluations;
            lower = allPlus.lowerBounds[0][0];
            upper = allPlus.upperBounds[0][0];

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0  || lower >= upper)
                resultPlus =  0.0;
            else
                resultPlus = gauss_legendre(64, functionToIntegrateGauss, &allPlus, lower, upper);



            allMinus.evaluations = evaluations;
            lower = allMinus.lowerBounds[0][0];
            upper = allMinus.upperBounds[0][0];

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0  || lower >= upper)
                resultMinus =  0.0;
            else
                resultMinus = gauss_legendre(64, functionToIntegrateGauss, &allMinus, lower, upper);

        }


        if (resultPlus > 1.0) {
            resultPlus = 1.0;
        }
        if (resultMinus > 1.0) {
            resultMinus = 1.0;
        }


        result = resultAll;
        if ((resultPlus - resultMinus) > 0) {
            result += (resultPlus - resultMinus);
        }

        return result;
	}



    double ProbabilityCalculator::functionToIntegrateGauss(double x, void* data){

   	    allDims all = *((allDims *)data);
        int n = all.evaluations;
   	    all.integrals[all.current_index].value = x;

        if (all.current_index  == (all.integrals.size() - 1)) {

            singleDim current;
            double result = 1.0;

            for (int i = 0; i < all.integrals.size(); i++) {

                current = all.integrals[i];
                result *= getDensity(current.distribution, current.value);

            }

            return result;

   		} else {

            all.current_index++;
            int index_next = all.current_index;


            singleDim currentDependency;
            double lower = all.lowerBounds[index_next][0];
            double upper = all.upperBounds[index_next][0];
            double lowerFactor;
            double upperFactor;

             for (int i = 0; i < index_next; i++) {
                 currentDependency = all.integrals[i];

                lowerFactor = 0.0;
                upperFactor = 0.0;

                if (all.lowerBounds[index_next].size() > i + 1)
                    lowerFactor = all.lowerBounds[index_next][i + 1];
                 if (all.lowerBounds[index_next].size() > i + 1)
                    upperFactor = all.upperBounds[index_next][i + 1];

                if (!isnan(lowerFactor) && lowerFactor != 0.0)
                    lower += (lowerFactor * currentDependency.value);

                if (!isnan(upperFactor)  && upperFactor != 0.0)
                    upper += (upperFactor * currentDependency.value);

            }

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0 )
                return 0.0;


   			return gauss_legendre(n, functionToIntegrateGauss, &all, lower, upper);
   		}

   	}



    double ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(const ParametricLocation &location, vector<pair<string, map<string, float>>> distributions, double maxTime, char algorithm, int functioncalls, double &error){


        double completeResult = 0.0;
        double result;


        for (std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals : location.getIntegrationIntervals()) {

            allDims all;
            allDims allPlus;
            allDims allMinus;

            all.current_index = 0;
            allPlus.current_index = 0;
            allMinus.current_index = 0;

          //  cout << "Max time: " << maxTime << " - ";

            //std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = location.getIntegrationIntervals();
            vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
            int dimension = location.getDimension();

            //cout << "GTF: " << generalTransitionsFired << " - ";
            //cout << "Accumulated Probability: " << location.getAccumulatedProbability() << endl << endl;

            for (int i = 0; i < integrationIntervals.size(); i++) {
                singleDim sAll;
                sAll.distribution = distributions[integrationIntervals[i].first];
                //sAll.distribution = distributions[0];
                all.integrals.push_back(sAll);
                singleDim sPlus;
                sPlus.distribution = sAll.distribution;
                allPlus.integrals.push_back(sPlus);
                singleDim sMinus;
                sMinus.distribution = sAll.distribution;
                allMinus.integrals.push_back(sMinus);


                all.lowerBounds.push_back(integrationIntervals[i].second.first);
                all.upperBounds.push_back(integrationIntervals[i].second.second);
                allPlus.lowerBounds.push_back(integrationIntervals[i].second.first);
                allPlus.upperBounds.push_back(integrationIntervals[i].second.second);
                allMinus.lowerBounds.push_back(integrationIntervals[i].second.first);
                allMinus.upperBounds.push_back(integrationIntervals[i].second.second);

                if (isinf(integrationIntervals[i].second.second[0])) {

                       int last = allPlus.upperBounds.size() - 1;

                       fill(all.upperBounds[last].begin(), all.upperBounds[last].end(), 0.0);
                       all.upperBounds[last][0] = maxTime;


                       //Faking a distribution with probability 1
                       fill(allPlus.upperBounds[last].begin(), allPlus.upperBounds[last].end(), 0.0);
                       allPlus.upperBounds[last][0] = maxTime;
                       fill(allPlus.lowerBounds[last].begin(), allPlus.lowerBounds[last].end(), 0.0);
                       pair<string, map<string, float>> uniform;
                       uniform.first = "uniform";
                       std::map<string, float> params;
                       params["a"] = 0.0;
                       params["b"] = maxTime;
                       uniform.second = params;
                       allPlus.integrals[last].distribution = uniform;

                       fill(allMinus.upperBounds[last].begin(), allMinus.upperBounds[last].end(), 0.0);
                       allMinus.upperBounds[last][0] = maxTime;
                       fill(allMinus.lowerBounds[last].begin(), allMinus.lowerBounds[last].end(), 0.0);
                   }

             //  cout << "TransitionID:" << integrationIntervals[i].first << " in [ " << integrationIntervals[i].second.first << " ; " << integrationIntervals[i].second.second << " ] " << endl;
            }


            result = computeMultivariateIntegralUsingMonteCarlo(functioncalls, all, allPlus,  allMinus, algorithm, error);


            completeResult += result;
            //cout << "Local Result " << result << endl;
        }


        return completeResult;
   	}



    double ProbabilityCalculator::getProbabilityForPolytopeUsingMonteCarlo(
        const hypro::HPolytope<double> &region,
        const vector<pair<string, map<string, float>>> &distributions,
        char algorithm, int functioncalls,
        double &error
    ) {
        if (region.empty() || region.dimension() == 0)
            return 0.0;

        auto vertices = region.vertices();
        if (vertices.empty())
            return 0.0;

        long maxDim = vertices.begin()->rawCoordinates().rows();
        hypro::matrix_t<double> matr = matrix_t<double>(vertices.size()-1, maxDim);
        // use first vertex as origin, start at second vertex
        long rowIndex = 0;
        for(auto vertexIt = ++vertices.begin(); vertexIt != vertices.end(); ++vertexIt, ++rowIndex) {
            matr.row(rowIndex) = (vertexIt->rawCoordinates() - vertices.begin()->rawCoordinates()).transpose();
        }
        auto effectiveDimension = int(matr.fullPivLu().rank());
        if (effectiveDimension < region.dimension()) {
            std::cout << "Polytope with zero volume since effective dimension < space dimension (" << effectiveDimension << " < " << region.dimension() << ")" << std::endl;
            return 0.0;
        }

        assert(distributions.size() == region.dimension());
        double result = 0.0;

        for(const auto &simplex : Triangulation::create(region))
        {
            std::vector<std::pair<std::vector<double>, std::vector<double>>> integrationIntervals = simplex.getIntegrationIntervals();

            allDims all;
            allDims allPlus;
            allDims allMinus;

            all.current_index = 0;
            all.applyTransformation = true;
            all.transformationMatrix = simplex.getIntegrationTransformationMatrix();
            allPlus.current_index = 0;
            allPlus.applyTransformation = true;
            allPlus.transformationMatrix = simplex.getIntegrationTransformationMatrix();
            allMinus.current_index = 0;
            allMinus.applyTransformation = true;
            allMinus.transformationMatrix = simplex.getIntegrationTransformationMatrix();


            for (int i = 0; i < integrationIntervals.size(); i++) {
                singleDim sAll;
                sAll.distribution = distributions[i];
                all.integrals.push_back(sAll);
                singleDim sPlus;
                sPlus.distribution = sAll.distribution;
                allPlus.integrals.push_back(sPlus);
                singleDim sMinus;
                sMinus.distribution = sAll.distribution;
                allMinus.integrals.push_back(sMinus);

                all.lowerBounds.push_back(integrationIntervals[i].first);
                all.upperBounds.push_back(integrationIntervals[i].second);
                allPlus.lowerBounds.push_back(integrationIntervals[i].first);
                allPlus.upperBounds.push_back(integrationIntervals[i].second);
                allMinus.lowerBounds.push_back(integrationIntervals[i].first);
                allMinus.upperBounds.push_back(integrationIntervals[i].second);

                if (isinf(integrationIntervals[i].second[0])) {
                    int last = allPlus.upperBounds.size() - 1;

                    fill(all.upperBounds[last].begin(), all.upperBounds[last].end(), 0.0);
                    all.upperBounds[last][0] = 1.0;

                    // Faking a distribution with probability 1
                    fill(allPlus.upperBounds[last].begin(), allPlus.upperBounds[last].end(), 0.0);
                    allPlus.upperBounds[last][0] = 1.0;
                    fill(allPlus.lowerBounds[last].begin(), allPlus.lowerBounds[last].end(), 0.0);
                    const auto &uniform = std::pair<std::string, std::map<std::string, float>>{"uniform", {{"a", 0.0}, {"b", 0.0}}};
                    allPlus.integrals[last].distribution = uniform;

                    fill(allMinus.upperBounds[last].begin(), allMinus.upperBounds[last].end(), 0.0);
                    allMinus.upperBounds[last][0] = 1.0;
                    fill(allMinus.lowerBounds[last].begin(), allMinus.lowerBounds[last].end(), 0.0);
                }
            }


            result += computeMultivariateIntegralUsingMonteCarlo(functioncalls, all, allPlus,  allMinus, algorithm, error);
        }


        return result;
    }



    double ProbabilityCalculator::getProbabilityForIntersectionOfPolytopesUsingMonteCarlo(
        const vector<HPolytope<double>> &polytopes,
        const vector<pair<string, map<string, float>>> &distributionsNormalized,
        char algorithm,
        int functioncalls,
        double &error
    ){
        HPolytope<double> regionToIntegrate{polytopes[0]};

       if (polytopes.size() > 1)
           for(HPolytope<double> currentRegion : polytopes)
               regionToIntegrate = regionToIntegrate.intersect(currentRegion);

       return ProbabilityCalculator::getProbabilityForPolytopeUsingMonteCarlo(
           regionToIntegrate,
           distributionsNormalized,
           algorithm,
           functioncalls,
           error
       );
    }



    double ProbabilityCalculator::getProbabilityForUnionOfPolytopesUsingMonteCarlo(
        std::vector<HPolytope<double>> polytopes,
        const vector<pair<string, map<string, float>>> &distributionsNormalized,
        char algorithm,
        int functioncalls,
        double &error
    ){

        std::cout << "ProbabilityCalculator: polytopes.size before erase" << polytopes.size() << std::endl;
        polytopes.erase(
                std::remove_if(polytopes.begin(), polytopes.end(), [](HPolytope<double> region) {
                    if (region.empty() || region.dimension() == 0) {
                    return true;
                }

                    auto vertices = region.vertices();
                    if (vertices.empty()) {
                        return true;
                    }
                    long maxDim = vertices.begin()->rawCoordinates().rows();
                    std::cout << "vertices.size "<< vertices.size() << std::endl;
                    std::cout << "maxDim " << maxDim<< std::endl;
                    hypro::matrix_t<double> matr = matrix_t<double>(vertices.size()-1, maxDim);
                    // use first vertex as origin, start at second vertex
                    std::cout << "effectiveDim " << int(matr.fullPivLu().rank()) << std::endl;
                    long rowIndex = 0;
                    for(auto vertexIt = ++vertices.begin(); vertexIt != vertices.end(); ++vertexIt, ++rowIndex) {
                        matr.row(rowIndex) = (vertexIt->rawCoordinates() - vertices.begin()->rawCoordinates()).transpose();
                    }
                    auto effectiveDimension = int(matr.fullPivLu().rank());
                    std::cout << "effectiveDim " << effectiveDimension << std::endl;
                    std::cout << "region.Dimension() " << region.dimension() << std::endl;
                    return effectiveDimension < region.dimension();
                }),
                polytopes.end()
        );

        double probability = 0.0;
        error = 0.0;
        double currentError;
        double currentProb;
        double p = 1.0;
        unsigned long n = polytopes.size();
        std::cout << "ProbabilityCalculator: polytopes.size after erase " << n << std::endl;
        for (int k = 1; k <= polytopes.size(); ++k){

            vector<bool> v(n);
            fill(v.begin(), v.begin() + k, true);

            do {

                vector<hypro::HPolytope<double>> regionsToIntegrate;

                for (int i = 0; i < n; ++i) {
                    if (v[i]) {
                        regionsToIntegrate.push_back(polytopes[i]);
                    }
                }

                currentError = 0.0;
                currentProb = getProbabilityForIntersectionOfPolytopesUsingMonteCarlo(
                    regionsToIntegrate,
                    distributionsNormalized,
                    algorithm,
                    functioncalls,
                    currentError
                );

                probability += p * currentProb;
                error += currentError;
                std::cout << "ProbabilityCalculator: currentProb " << currentProb << std::endl;
                std::cout << "ProbabilityCalculator: probability " << probability << std::endl;
                std::cout << "ProbabilityCalculator: currentError " << currentError << std::endl;
                std::cout << "ProbabilityCalculator: error " << error << std::endl;

            } while (std::prev_permutation(v.begin(), v.end()));

            p *= -1.0;
        }

        return probability;


    }



    double ProbabilityCalculator::computeMultivariateIntegralUsingMonteCarlo(int functioncalls, hpnmg::allDims all, hpnmg::allDims allPlus, hpnmg::allDims allMinus, char algorithm, double &error){


        double result = 0.0;

        double resultAll = 0.0;
        double resultPlus = 0.0;
        double resultMinus = 0.0;

        double errorAll = 0.0;
        double errorPlus = 0.0;
        double errorMinus = 0.0;



        const int dim = all.integrals.size();


        //Integration

        if (dim > 0) {


            double *xl = new double[dim];
            double *xu = new double[dim];


            for (int i = 0; i < dim; i++) {
                xl[i] = -1.0;
                xu[i] = 1.0;
            }

            size_t dimension1 = static_cast<size_t>(dim);


            gsl_monte_function G = {&transformedFunctionToIntegrateMonteCarlo, dimension1, &all};
            gsl_monte_function GPlus = {&transformedFunctionToIntegrateMonteCarlo, dimension1, &allPlus};
            gsl_monte_function GMinus = {&transformedFunctionToIntegrateMonteCarlo, dimension1, &allMinus};

            size_t calls = static_cast<size_t>(functioncalls);

            const gsl_rng_type *T;
            gsl_rng *r;



            //All
            gsl_rng_env_setup();

            T = gsl_rng_default;
            r = gsl_rng_alloc(T);

            if (algorithm == 1) {

                gsl_monte_plain_state *s = gsl_monte_plain_alloc(dimension1);
                gsl_monte_plain_integrate(&G, xl, xu, dimension1, calls, r, s, &resultAll, &errorAll);
                gsl_monte_plain_free(s);

                //cout << "Plain Monte Carlo integral result: " << resultAll << ", " << "error estimate: " << errorAll << endl;

            } else if (algorithm == 2) {

                gsl_monte_miser_state *s = gsl_monte_miser_alloc(dimension1);
                gsl_monte_miser_integrate(&G, xl, xu, dimension1, calls, r, s, &resultAll, &error);
                gsl_monte_miser_free(s);
                //cout << "Monte Carlo MISER integral result: " << resultAll << ", " << "error estimate: " << errorAll << endl;

            } else if (algorithm == 3) {

                gsl_monte_vegas_state *s = gsl_monte_vegas_alloc(dimension1);

                //vegas warm-up
                gsl_monte_vegas_integrate(&G, xl, xu, dimension1, calls / 50, r, s, &resultAll, &errorAll);

                for (int j = 0; j < 10; j++) {
                    gsl_monte_vegas_integrate(&G, xl, xu, dimension1, calls / 5, r, s, &resultAll, &errorAll);

                    if ((fabs(gsl_monte_vegas_chisq(s) - 1.0) <= 0.5) || (error == 0.0))
                        break;
                }
                gsl_monte_vegas_free(s);

                if (fabs(gsl_monte_vegas_chisq(s) - 1.0) > 0.5 && errorAll > 0.0) {
                    //cout << "Monte Carlo VEGAS not converging, switched to MISER" << endl;
                    gsl_monte_miser_state *z = gsl_monte_miser_alloc(dimension1);
                    gsl_monte_miser_integrate(&G, xl, xu, dimension1, calls, r, z, &resultAll, &errorAll);
                    gsl_monte_miser_free(z);
                    //cout << "Monte Carlo MISER integral result: " << resultAll << ", " << "error estimate: " << errorAll << endl;
                } //else
                //cout << "Monte Carlo VEGAS final integral result: " << resultAll << ", " << "error estimate: " << errorAll << endl;
            }

            gsl_rng_free(r);







            //AllPlus: Correction for Infinity
            gsl_rng_env_setup();

            T = gsl_rng_default;
            r = gsl_rng_alloc(T);

            if (algorithm == 1) {

                gsl_monte_plain_state *s = gsl_monte_plain_alloc(dimension1);
                gsl_monte_plain_integrate(&GPlus, xl, xu, dimension1, calls, r, s, &resultPlus, &errorPlus);
                gsl_monte_plain_free(s);

                //cout << "Plain Monte Carlo integral result: " << resultPlus << ", " << "error estimate: " << errorPlus << endl;

            } else if (algorithm == 2) {

                gsl_monte_miser_state *s = gsl_monte_miser_alloc(dimension1);
                gsl_monte_miser_integrate(&GPlus, xl, xu, dimension1, calls, r, s, &resultPlus, &error);
                gsl_monte_miser_free(s);
                //cout << "Monte Carlo MISER integral result: " << resultPlus << ", " << "error estimate: " << errorPlus << endl;

            } else if (algorithm == 3) {

                gsl_monte_vegas_state *s = gsl_monte_vegas_alloc(dimension1);

                //vegas warm-up
                gsl_monte_vegas_integrate(&GPlus, xl, xu, dimension1, calls / 50, r, s, &resultPlus, &errorPlus);

                for (int j = 0; j < 10; j++) {
                    gsl_monte_vegas_integrate(&GPlus, xl, xu, dimension1, calls / 5, r, s, &resultPlus, &errorPlus);

                    if ((fabs(gsl_monte_vegas_chisq(s) - 1.0) <= 0.5) || (errorPlus == 0.0))
                        break;
                }
                gsl_monte_vegas_free(s);

                if (fabs(gsl_monte_vegas_chisq(s) - 1.0) > 0.5 && errorPlus > 0.0) {
                    //cout << "Monte Carlo VEGAS not converging, switched to MISER" << endl;
                    gsl_monte_miser_state *z = gsl_monte_miser_alloc(dimension1);
                    gsl_monte_miser_integrate(&GPlus, xl, xu, dimension1, calls, r, z, &resultPlus, &errorPlus);
                    gsl_monte_miser_free(z);
                 //   cout << "Monte Carlo MISER integral result: " << resultPlus << ", " << "error estimate: " << errorPlus << endl;
                } //else
                //cout << "Monte Carlo VEGAS final integral result: " << resultPlus << ", " << "error estimate: " << errorPlus << endl;
            }

            gsl_rng_free(r);





            //AllMinus: Correction for Infinity
            gsl_rng_env_setup();

            T = gsl_rng_default;
            r = gsl_rng_alloc(T);

            if (algorithm == 1) {

                gsl_monte_plain_state *s = gsl_monte_plain_alloc(dimension1);
                gsl_monte_plain_integrate(&GMinus, xl, xu, dimension1, calls, r, s, &resultMinus, &errorMinus);
                gsl_monte_plain_free(s);

                //cout << "Plain Monte Carlo integral result: " << resultMinus << ", " << "error estimate: " << errorMinus << endl;

            } else if (algorithm == 2) {

                gsl_monte_miser_state *s = gsl_monte_miser_alloc(dimension1);
                gsl_monte_miser_integrate(&GMinus, xl, xu, dimension1, calls, r, s, &resultMinus, &error);
                gsl_monte_miser_free(s);
                //cout << "Monte Carlo MISER integral result: " << resultMinus << ", " << "error estimate: " << errorMinus << endl;

            } else if (algorithm == 3) {

                gsl_monte_vegas_state *s = gsl_monte_vegas_alloc(dimension1);

                //vegas warm-up
                gsl_monte_vegas_integrate(&GMinus, xl, xu, dimension1, calls / 50, r, s, &resultMinus, &errorMinus);

                for (int j = 0; j < 10; j++) {
                    gsl_monte_vegas_integrate(&GMinus, xl, xu, dimension1, calls / 5, r, s, &resultMinus,
                                              &errorMinus);

                    if ((fabs(gsl_monte_vegas_chisq(s) - 1.0) <= 0.5) || (errorMinus == 0.0))
                        break;
                }
                gsl_monte_vegas_free(s);

                if (fabs(gsl_monte_vegas_chisq(s) - 1.0) > 0.5 && errorMinus > 0.0) {
                    //cout << "Monte Carlo VEGAS not converging, switched to MISER" << endl;
                    gsl_monte_miser_state *z = gsl_monte_miser_alloc(dimension1);
                    gsl_monte_miser_integrate(&GMinus, xl, xu, dimension1, calls, r, z, &resultMinus, &errorMinus);
                    gsl_monte_miser_free(z);
                    //cout << "Monte Carlo MISER integral result: " << resultMinus << ", " << "error estimate: " << errorMinus << endl;
                } //else
                //cout << "Monte Carlo VEGAS final integral result: " << resultMinus << ", " << "error estimate: " << errorMinus << endl;
            }

            gsl_rng_free(r);


            delete[] xl;
            delete[] xu;

        }

        if (resultPlus > 1.0) {
            errorPlus -= (resultPlus - 1.0);
            resultPlus = 1.0;
        }
        if (resultMinus > 1.0) {
            errorMinus -= (resultMinus - 1.0);
            resultMinus = 1.0;
        }


        result = resultAll;
        error = errorAll;

        if ((resultPlus - resultMinus) > 0) {
            result += (resultPlus - resultMinus);
            error += (errorPlus + errorMinus);
        }


        return result;
    }

    double ProbabilityCalculator::transformedFunctionToIntegrateMonteCarlo(double *k, size_t dim, void* params){
        allDims all = *((allDims *)params);
        double result = 1.0;

        const auto values = Eigen::Map<Eigen::VectorXd>(k, dim);
        auto transformedValues = Eigen::VectorXd(dim);
        Eigen::VectorXd jacobianFactors = Eigen::VectorXd::Ones(dim);

        // Apply the transformation from the Monte Carlo hypercube into the volume defined by the intervals
        for (int i = 0; i < dim; i++) {
            double lower = all.lowerBounds[i][0];
            double upper = all.upperBounds[i][0];

            if (!isinf(upper)) {
                for (int j = 0; j < i; j++) {
                    double lowerFactor = 0.0;
                    double upperFactor = 0.0;

                    if (all.lowerBounds[i].size() > j + 1)
                        lowerFactor = all.lowerBounds[i][j + 1];
                    if (all.lowerBounds[i].size() > j + 1)
                        upperFactor = all.upperBounds[i][j + 1];

                    if (!isnan(lowerFactor) && lowerFactor != 0.0)
                        lower += (lowerFactor * transformedValues[j]);

                    if (!isnan(upperFactor) && upperFactor != 0.0)
                        upper += (upperFactor * transformedValues[j]);
                }

                if ((lower > 0.0 && isinf(lower)) || upper <= 0.0)
                    return 0.0;

                double halfLower = 0.5 * lower;
                double halfUpper = 0.5 * upper;

                transformedValues[i] = halfUpper + halfLower + k[i] * (halfUpper - halfLower);
                jacobianFactors[i] *= (halfUpper - halfLower);
            }
        }

        if (all.applyTransformation) {
            transformedValues = (all.transformationMatrix * transformedValues.homogeneous()).hnormalized();
            result *= std::abs(all.transformationMatrix.determinant());
        }

        for (int i = 0; i < dim; i++) {
            singleDim current = all.integrals[i];
            double upper = all.upperBounds[i][0];

            if (!isinf(upper)) {
                double currentResult = getDensity(current.distribution, transformedValues[i]);

                if (currentResult == 0.0)
                    return 0.0;

                double currentResultWithJacobian = currentResult * jacobianFactors[i];
                result *= currentResultWithJacobian;
            }
            else //Infinity case
                result *= 0.5;
        }

        return result;

    }



    double ProbabilityCalculator::getDensity(pair<string, map<string, float>> distribution, double value){

   		map<string, float> normalMap = distribution.second;

           if (isinf(value) || isnan(value))
               return 0.0;

   		try{
   			if (distribution.first == "normal"){

                double sigma = (double) normalMap.at("sigma");

                if (sigma <= 0.0) {
                    cout << "Invalid normal distribution. Make sure that sigma > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::normal dist ((double) normalMap.at("mu"), sigma);
                return (boost::math::pdf(dist, value));


            } else  if (distribution.first == "foldednormal"){

                double sigma = (double) normalMap.at("sigma");
                double mu = (double) normalMap.at("mu");

                if (sigma <= 0.0 || mu < 0.0) {
                    cout << "Invalid folded normal distribution. Make sure that sigma > 0 and mu >= 0.0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::normal dist (mu, sigma);
                return (boost::math::pdf(dist, value) + boost::math::pdf(dist, (-1.0)*value));


   			} else  if (distribution.first == "uniform"){

   				double a = (double) normalMap.at("a");
   				double b = (double) normalMap.at("b");

   				if (b <= a) {
   					cout << "Invalid uniform distribution. Make sure that a > b." << endl;
   					throw std::invalid_argument("invalid distribution");
   				}

   				if (value < a || value > b)
   					return 0.0;

   				boost::math::uniform dist (a,b);
   				return boost::math::pdf(dist, value);


   			} else  if (distribution.first == "exp"){

   			    double lambda;
   			    double mean = 0.0;

                if (normalMap.find("lambda") == normalMap.end() ) {
                    mean = (double) normalMap.at("mean");
                    lambda = 1.0 / mean;
                } else {
                    lambda = (double) normalMap.at("lambda");
                }

                if (lambda <= 0.0 && mean <= 0.0) {
                    cout << "Invalid exponential distribution. Make sure that either lambda > 0 or mean > 0 is given." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

   				boost::math::exponential dist (lambda);
   			    return boost::math::pdf(dist, value);


            } else  if (distribution.first == "halfnormal"){

                double sigma = (double) normalMap.at("sigma");

                if (sigma <= 0.0) {
                    cout << "Invalid half normal distribution. Make sure that sigma > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

               boost::math::normal dist (0.0, sigma);
               return (boost::math::pdf(dist, value) + boost::math::pdf(dist, (-1.0)*value));


            } else  if (distribution.first == "lognormal"){

                double sigma = (double) normalMap.at("sigma");

                if (sigma <= 0.0) {
                    cout << "Invalid log normal distribution. Make sure that sigma > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

               boost::math::lognormal dist ((double) normalMap.at("mu"), sigma);
               return boost::math::pdf(dist, value);


            } else  if (distribution.first == "inversenormal"){

                double mu = (double) normalMap.at("mu");
                double lambda = (double) normalMap.at("lambda");

                if (mu <= 0.0 || lambda <=0.0) {
                    cout << "Invalid inverse normal distribution. Make sure that mu > 0 and lambda > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

               boost::math::inverse_gaussian dist (mu, lambda);
               return boost::math::pdf(dist, value);


            } else  if (distribution.first == "beta"){

                double alpha = (double) normalMap.at("alpha");
                double beta = (double) normalMap.at("beta");

                if (alpha <= 0.0 || beta <=0.0) {
                    cout << "Invalid beta distribution. Make sure that alpha > 0 and beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

               boost::math::beta_distribution<double> dist (alpha, beta);
               return boost::math::pdf(dist, value);


            } else  if (distribution.first == "cauchy"){

                double beta = (double) normalMap.at("beta");

                if (beta <=0.0) {
                    cout << "Invalid cauchy distribution. Make sure that beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::cauchy dist ((double) normalMap.at("alpha"), beta);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "chisquare"){

                int n = (int) normalMap.at("n");

                if (n <= 0) {
                    cout << "Invalid chi square distribution. Make sure that n is a natural number > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::chi_squared dist (n);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "fisherf"){

                int n = (int) normalMap.at("n");
                int m = (int) normalMap.at("m");

                if (n <= 0 || m <= 0) {
                    cout << "Invalid fisher f distribution. Make sure that m and n are both natural numbers > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::fisher_f dist (n, m);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "gamma"){

                double alpha = (double) normalMap.at("alpha");
                double beta = (double) normalMap.at("beta");

                if (alpha <= 0.0 || beta <=0.0) {
                    cout << "Invalid gamma distribution. Make sure that alpha > 0 and beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::gamma_distribution<double> dist (alpha, (1.0/beta));
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "inversegamma"){

                double alpha = (double) normalMap.at("alpha");
                double beta = (double) normalMap.at("beta");

                if (alpha <= 0.0 || beta <=0.0) {
                    cout << "Invalid inverse gamma distribution. Make sure that alpha > 0 and beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::inverse_gamma dist (alpha, beta);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "laplace"){

                double beta = (double) normalMap.at("beta");

                if (beta <=0.0) {
                    cout << "Invalid laplace distribution. Make sure that beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::laplace dist ((double) normalMap.at("mu"), beta);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "logistic"){

                double s = (double) normalMap.at("s");

                if (s <= 0.0) {
                    cout << "Invalid log normal distribution. Make sure that s > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

               boost::math::logistic dist ((double) normalMap.at("mu"), s);
               return boost::math::pdf(dist, value);


            } else  if (distribution.first == "pareto"){

                double alpha = (double) normalMap.at("alpha");
                double beta = (double) normalMap.at("beta");

                if (alpha <= 0.0 || beta <=0.0) {
                    cout << "Invalid pareto distribution. Make sure that alpha > 0 and beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::pareto dist (alpha, beta);
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "rayleigh"){

                double beta = (double) normalMap.at("beta");

                if (beta <=0.0) {
                    cout << "Invalid laplace distribution. Make sure that beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::rayleigh dist (beta);
                return boost::math::pdf(dist, value);

            } else  if (distribution.first == "student"){

                int n = (int) normalMap.at("n");

                if (n <= 0) {
                    cout << "Invalid student distribution. Make sure that n is a natural number > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::students_t dist (n);
                return boost::math::pdf(dist, value);

            } else  if (distribution.first == "weibull"){

                double alpha = (double) normalMap.at("alpha");
                double beta = (double) normalMap.at("beta");

                if (alpha <= 0.0 || beta <=0.0) {
                    cout << "Invalid weibull distribution. Make sure that alpha > 0 and beta > 0." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                boost::math::weibull dist (alpha, beta);
                return boost::math::pdf(dist, value);


   			} else {
   				cout << "ERROR" << endl;
   				throw std::invalid_argument( "invalid distribution" );
   			}

   		} catch (const std::invalid_argument& e) {
   			cout << "ERROR"<< endl;
   			throw std::invalid_argument( "invalid distribution" );
   		}
   	}
}