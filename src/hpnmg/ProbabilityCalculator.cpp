#include "ProbabilityCalculator.h"

using namespace std;

namespace hpnmg {


ProbabilityCalculator::ProbabilityCalculator(){}


	typedef struct {
		double value;
		pair<string, map<string, float>> distribution;
	} singleDim;



	typedef struct {
		int current_index;
		vector<singleDim> integrals;
		vector<vector<double>> lowerBounds;
		vector<vector<double>> upperBounds;
	} allDims;




    double ProbabilityCalculator::getProbability(const vector<ParametricLocationTree::Node> &nodes, const ParametricLocationTree &tree, double timepoint){

        double total = 0.0;

        //TODO multiply with accumulated probability
        for (int i = 0;i < nodes.size(); i++)
            total +=  calculateIntervals(nodes[i].getParametricLocation(), tree, timepoint); // * nodes[i].getParametricLocation().getAccumulatedProbability();

        std::cout.precision(17);
        cout << "nodes size = " << nodes.size() << endl;
        cout << "total = " << total << ", n = " << n << endl;
        std::cout.precision(6);

        return total;

    }




	double ProbabilityCalculator::calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint){


		double result = 0.0;


		allDims all;
		all.current_index = 0;
		int dimension = location.getDimension();

		const vector<pair<string, map<string, float>>> distributions = tree.getDistributions();

		vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
		vector<bool> enabledTransitions = location.getGeneralTransitionsEnabled();

		vector<int> counter = vector<int>(location.getDimension() - 1);
		fill(counter.begin(), counter.end(),0);


		//firings in the past
		for (int i = 0; i < generalTransitionsFired.size(); i++){

			int transitionID = generalTransitionsFired[i];
			int firing = counter[transitionID];

			singleDim s;
			s.distribution = distributions[transitionID];
			all.integrals.push_back(s);

			all.lowerBounds.push_back(location.getGeneralIntervalBoundLeft()[transitionID][firing]);
			all.upperBounds.push_back(location.getGeneralIntervalBoundRight()[transitionID][firing]);

			counter[transitionID] +=1;

		}




		//firings in the future (particular next firing per general transition)
		//lower interval bound = lowerBounds + t - entrance time, having lowerBounds and entrance time (from SourceEvent) as vectors dependent on random variables
		for (int j = 0; j < counter.size(); j++){

			int firing = counter[j];

			singleDim s;
			s.distribution = distributions[j];
			all.integrals.push_back(s);

			all.lowerBounds.push_back(location.getGeneralIntervalBoundLeft()[j][firing]);
			all.upperBounds.push_back(location.getGeneralIntervalBoundRight()[j][firing]);

			if (enabledTransitions[j] == true){

				all.lowerBounds[all.integrals.size() - 1][0] += location.getSourceEvent().getTime();

				vector<double> dependencies = location.getSourceEvent().getGeneralDependencies();
				for (int k = 1; k < dependencies.size(); k++)
					all.lowerBounds[all.integrals.size() - 1][k] += dependencies[k];

			}
		}


		if (all.integrals.size() > 0){
			double lower = all.lowerBounds[0][0];
			double upper = all.upperBounds[0][0];

            if (lower > 0.0 && isinf(lower))
                result =  0.0;
            else if(upper <= 0.0)
                result =  0.0;
            else
                result = correctValue(gauss_legendre(n, f, &all, lower, checkInfinity(upper)));

			cout << "integral result: " << result << endl;
		}

		return result;
	}



    double ProbabilityCalculator::f(double x, void* data){

   	    singleDim current;
           allDims all = *((allDims *)data);
   	    all.integrals[all.current_index].value = correctValue(x);


   		if (all.current_index  == (all.integrals.size() - 1)){

   			double result = 1.0;

   			for (int i = 0; i < all.integrals.size(); i++){
   				current = all.integrals[i];
   				result *= getDensity(current.distribution, current.value);
   			}

   			return result;

   		} else {

   			all.current_index++;
   			int index_next = all.current_index;

   			double lower = all.lowerBounds[index_next][0];
   			double upper = all.upperBounds[index_next][0];

   			for (int i = 0; i < index_next; i++){
   				current = all.integrals[i];

   				double lowerFactor = all.lowerBounds[index_next][i+1];
                   double upperFactor = all.upperBounds[index_next][i+1];

   				if (!isnan(lowerFactor))
                       lower += (correctValue(lowerFactor) * correctValue(current.value));

                   if (!isnan(upperFactor))
   				    upper += (correctValue(upperFactor) * correctValue(current.value));

   			}


               if (lower > 0.0 && isinf(lower))
                   return 0.0;

               if(upper <= 0.0)
                   return 0.0;

   			return gauss_legendre(n, f, &all, lower, checkInfinity(upper));
   		}

   	}



    double ProbabilityCalculator::checkInfinity(double value){

   		if (isinf(value))
   			return DBL_MAX;

   		return value;
   	}



   	double ProbabilityCalculator::correctValue(double value){

        if (value <= 0.0)//pow(10, -200))
           return 0.0;

       return value;
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