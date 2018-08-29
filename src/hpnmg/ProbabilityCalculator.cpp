#include "ProbabilityCalculator.h"

using namespace std;

namespace hpnmg {


ProbabilityCalculator::ProbabilityCalculator(){}


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
	} allDims;



    double ProbabilityCalculator::getProbabilityGauss(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int evaluations){

        double placeholder;
        return getProbability(nodes, tree,  timepoint, 0, 0, evaluations, placeholder);

    }



    double ProbabilityCalculator::getProbabilityMonteCarloPlain(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

        return getProbability(nodes, tree,  timepoint, 1, functioncalls, 0, totalerror);
    }



    double ProbabilityCalculator::getProbabilityMonteCarloMiser(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

        return getProbability(nodes, tree,  timepoint, 2, functioncalls, 0, totalerror);
    }



    double ProbabilityCalculator::getProbabilityMonteCarloVegas(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint, int functioncalls, double &totalerror){

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

            cout << "Left " << nodes[i].getParametricLocation().getGeneralIntervalBoundLeft() << endl;
            cout << "Right " << nodes[i].getParametricLocation().getGeneralIntervalBoundRight() << endl;
            cout << "Normed " << nodes[i].getParametricLocation().getGeneralDependenciesNormed() << endl;

            if (algorithm == 0)
                //Gauss Legendre
                nodeResult = calculateIntervalsGauss(nodes[i].getParametricLocation(), tree, timepoint, nodes[i].getNodeID(), evaluations) * nodes[i].getParametricLocation().getAccumulatedProbability();
            else
                //Monte Carlo Plain
                nodeResult = calculateIntervalsMonteCarlo(nodes[i].getParametricLocation(), tree, timepoint, nodes[i].getNodeID(), algorithm, functioncalls, error) * nodes[i].getParametricLocation().getAccumulatedProbability();

            total += nodeResult;
            totalerror += error;
        }

        return total;

    }



	double ProbabilityCalculator::calculateIntervalsGauss(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint, int nodeID, int evaluations){


		double result = 0.0;


		allDims all;
		all.current_index = 0;
        const vector<pair<string, map<string, float>>> distributions = tree.getDistributions();

        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = location.getIntegrationIntervals();
		for (int i = 0; i < integrationIntervals.size(); i++) {
            singleDim s;
            s.distribution = distributions[integrationIntervals[i].first];
            all.integrals.push_back(s);
            all.lowerBounds.push_back(integrationIntervals[i].second.first);
            all.upperBounds.push_back(integrationIntervals[i].second.second);

            cout << "Left bound:" << integrationIntervals[i].second.first << endl;
            cout << "Right bound:" << integrationIntervals[i].second.second << endl;
		}

		if (all.integrals.size() > 0){
		    all.evaluations = evaluations;
			double lower = all.lowerBounds[0][0];
			double upper = all.upperBounds[0][0];

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0  || lower >= upper)
                result =  0.0;
            else
                result = gauss_legendre(64, functionToIntegrateGauss, &all, lower, upper);


			cout << "Gauss integral result: " << result << endl;
            cout << "----" << endl;
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

                if (!isnan(upperFactor)  && lowerFactor != 0.0)
                    upper += (upperFactor * currentDependency.value);

            }

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0 )
                return 0.0;



   			return gauss_legendre(n, functionToIntegrateGauss, &all, lower, upper);
   		}

   	}



    double ProbabilityCalculator::calculateIntervalsMonteCarlo(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint, int nodeID, char algorithm, int functioncalls, double &error){


        double result = 0.0;


        allDims all;
        all.current_index = 0;
        const vector<pair<string, map<string, float>>> distributions = tree.getDistributions();

        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = location.getIntegrationIntervals();
        for (int i = 0; i < integrationIntervals.size(); i++) {
            singleDim s;
            s.distribution = distributions[integrationIntervals[i].first];
            all.integrals.push_back(s);
            all.lowerBounds.push_back(integrationIntervals[i].second.first);
            all.upperBounds.push_back(integrationIntervals[i].second.second);

            cout << "Left bound:" << integrationIntervals[i].second.first << endl;
            cout << "Right bound:" << integrationIntervals[i].second.second << endl;
        }



   		const int dim = all.integrals.size();


   		if (dim > 0){


            double* xl = new double[dim];
            double* xu = new double[dim];


            for (int i = 0;i < dim; i++){
                xl[i] = -1.0;
                xu[i] = 1.0;
            }

            size_t dimension = static_cast<size_t>(dim);

            const gsl_rng_type *T;
            gsl_rng *r;

            gsl_monte_function G = { &transformedFunctionToIntegrateMonteCarlo, dimension, &all };

            size_t calls = static_cast<size_t>(functioncalls);

            gsl_rng_env_setup ();

            T = gsl_rng_default;
            r = gsl_rng_alloc (T);


            if (algorithm == 1){

              gsl_monte_plain_state *s = gsl_monte_plain_alloc (dimension);
              gsl_monte_plain_integrate (&G, xl, xu, dimension, calls, r, s, &result, &error);
              gsl_monte_plain_free (s);

                cout << "Plain Monte Carlo integral result: " << result << ", " << "error estimate: " << error << endl;

            } else if (algorithm == 2){

              gsl_monte_miser_state *s = gsl_monte_miser_alloc (dimension);
              gsl_monte_miser_integrate (&G, xl, xu, dimension, calls, r, s, &result, &error);
              gsl_monte_miser_free (s);
              cout << "Monte Carlo MISER integral result: " << result << ", " << "error estimate: " << error << endl;

            } else if (algorithm == 3){

                gsl_monte_vegas_state *s = gsl_monte_vegas_alloc (dimension);

                //vegas warm-up
                gsl_monte_vegas_integrate (&G, xl, xu, dimension, calls/50, r, s, &result, &error);

                for (int j = 0; j < 10; j++) {
                    gsl_monte_vegas_integrate(&G, xl, xu, dimension, calls/5, r, s, &result, &error);

                    if ((fabs (gsl_monte_vegas_chisq (s) - 1.0) <= 0.5) || (error == 0.0))
                        break;
                }
                gsl_monte_vegas_free (s);

                if (fabs (gsl_monte_vegas_chisq (s) - 1.0) > 0.5 && error > 0.0){
                    cout << "Monte Carlo VEGAS not converging, switched to MISER" << endl;
                    gsl_monte_miser_state *z = gsl_monte_miser_alloc (dimension);
                    gsl_monte_miser_integrate (&G, xl, xu, dimension, calls, r, z, &result, &error);
                    gsl_monte_miser_free (z);
                    cout << "Monte Carlo MISER integral result: " << result << ", " << "error estimate: " << error << endl;
                } else
                    cout << "Monte Carlo VEGAS final integral result: " << result << ", " << "error estimate: " << error << endl;
            }

            gsl_rng_free (r);


            delete[] xl;
            delete[] xu;

   		}

   		return result;
   	}



    double ProbabilityCalculator::transformedFunctionToIntegrateMonteCarlo(double *k, size_t dim, void* params){



   	    allDims all = *((allDims *)params);
        double result = 1.0;

        singleDim current;
        singleDim currentDependency;
        double lower;
        double upper;
        double lowerFactor;
        double upperFactor;

        double* transformedValues = new double[dim];



        for (int i = 0; i < dim; i++){

            current = all.integrals[i];
            lower = all.lowerBounds[i][0];
            upper = all.upperBounds[i][0];


            for (int j = 0; j < i; j++) {
                currentDependency = all.integrals[j];

                lowerFactor = 0.0;
                upperFactor = 0.0;

                if (all.lowerBounds[i].size() > j + 1)
                    lowerFactor = all.lowerBounds[i][j + 1];
                if (all.lowerBounds[i].size() > j + 1)
                    upperFactor = all.upperBounds[i][j + 1];

                if (!isnan(lowerFactor) && lowerFactor != 0.0)
                    lower += (lowerFactor * transformedValues[j]);

               if (!isnan(upperFactor)  && lowerFactor != 0.0)
                    upper += (upperFactor * transformedValues[j]);

           }


            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0 )
                return 0.0;

            transformedValues[i] = 0.5 * (upper + lower) + k[i] * 0.5 * (upper - lower);
            result *= getDensity(current.distribution, transformedValues[i]) * 0.5 *(lower - upper);
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

                double lambda = (double) normalMap.at("lambda");

                if (lambda <= 0.0) {
                    cout << "Invalid exponential distribution. Make sure that lambda > 0." << endl;
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