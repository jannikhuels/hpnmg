#include "ProbabilityCalculator.h"

using namespace std;

namespace hpnmg {


ProbabilityCalculator::ProbabilityCalculator(){}


	typedef struct {
		double value;
		pair<string, map<string, float>> distribution;
		bool fired = false;
	} singleDim;



	typedef struct {
		int current_index;
		vector<singleDim> integrals;
		vector<vector<double>> lowerBounds;
		vector<vector<double>> upperBounds;
        vector<vector<double>> upperBoundThresholds;

	} allDims;




    double ProbabilityCalculator::getProbability(vector<ParametricLocationTree::Node> &nodes, ParametricLocationTree &tree, double timepoint){

        double total = 0.0;

        map<int, int> parents;
        map<int, vector<vector<double>>> thresholds;
        bool restricted;
        int parentID;

        for (int i = 0;i < nodes.size(); i++) {
            cout << "Node: " << nodes[i].getNodeID() << endl;

            parentID = -1.0;
            if (parents.count(nodes[i].getNodeID()) > 0)
                parentID = parents.find(nodes[i].getNodeID())->second ;


            total += calculateIntervals(nodes[i].getParametricLocation(), tree, timepoint, thresholds, restricted, nodes[i].getNodeID(), parentID) * nodes[i].getParametricLocation().getAccumulatedProbability();


            if (restricted) {
                vector<ParametricLocationTree::Node> children = tree.getChildNodes(nodes[i]);
                for (int j = 0; j < children.size(); j++){
                    parents.insert(pair<int, int>(children[j].getNodeID(), nodes[i].getNodeID()));
                }
            }
        }

        if (total > 0.999999999999999)
            total = 1.0;
        else if (total < 0.000000000000001)
            total = 0.0;


        return total;

    }




	double ProbabilityCalculator::calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree, double timepoint, map<int, vector<vector<double>>> &thresholds, bool &restricted, int nodeID, int parentID){


		double result = 0.0;
        restricted = false;

		allDims all;
		all.current_index = 0;

		const vector<pair<string, map<string, float>>> distributions = tree.getDistributions();

		vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
		vector<bool> enabledTransitions = location.getGeneralTransitionsEnabled();

		vector<int> counter = vector<int>(location.getDimension() - 1);
		fill(counter.begin(), counter.end(),0);

        vector<vector<double>> newThresholds;


		//firings in the past
		// upper interval bound = min (upperBounds, threshold from parent)
		for (int i = 0; i < generalTransitionsFired.size(); i++) {

            int transitionID = generalTransitionsFired[i];
            int firing = counter[transitionID];

            singleDim s;
            s.distribution = distributions[transitionID];
            s.fired = true;
            all.integrals.push_back(s);

            all.lowerBounds.push_back(location.getGeneralIntervalBoundLeft()[transitionID][firing]);
            all.upperBounds.push_back(location.getGeneralIntervalBoundRight()[transitionID][firing]);

			counter[transitionID] +=1;

		}
        if (parentID >= 0) {
            all.upperBoundThresholds = thresholds.find(parentID)->second;
            newThresholds = all.upperBoundThresholds;
        }


		//firings in the future (particular next firing per general transition)
		//lower interval bound = max(lowerBounds + t - entrance time, lower) having lowerBounds and entrance time (from SourceEvent) as vectors dependent on random variables
		for (int j = 0; j < counter.size(); j++){

            int firing = counter[j];

		    bool enablingTimeGreaterZero = false;
            for (int l = 0; l < location.getGeneralIntervalBoundLeft()[j][firing].size(); l++) {
                if (location.getGeneralIntervalBoundLeft()[j][firing][l] > 0)
                    enablingTimeGreaterZero = true;
            }

            if (enabledTransitions[j] || enablingTimeGreaterZero) {

                singleDim s;
                s.distribution = distributions[j];
                s.fired = false;
                all.integrals.push_back(s);

                all.lowerBounds.push_back(location.getGeneralIntervalBoundLeft()[j][firing]);
                all.upperBounds.push_back(location.getGeneralIntervalBoundRight()[j][firing]);

                if (enabledTransitions[j]) {

                    all.lowerBounds[all.integrals.size() - 1][0] += (timepoint - location.getSourceEvent().getTime());

                    vector<double> dependencies = location.getSourceEvent().getGeneralDependencies();
                    for (int l = 0; l < dependencies.size(); l++)
                        all.lowerBounds[all.integrals.size() - 1][l + 1] -= dependencies[l];

                    restricted = true;
                    if (newThresholds.size() > all.integrals.size() - 1)
                        newThresholds[all.integrals.size() - 1] = all.lowerBounds[all.integrals.size() - 1];
                    else
                        newThresholds.push_back( all.lowerBounds[all.integrals.size() - 1]);
                }
            }
		}

        if (restricted || parentID >= 0)
            thresholds.insert(pair<int, vector<vector<double>>>(nodeID, newThresholds));


		if (all.integrals.size() > 0){
			double lower = all.lowerBounds[0][0];
			double upper = all.upperBounds[0][0];

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0)
                result =  0.0;
            else {
                if (all.integrals[0].fired && all.upperBoundThresholds.size() > 0 && upper > all.upperBoundThresholds[0][0])
                    upper =  all.upperBoundThresholds[0][0];
                result = correctValue(gauss_legendre(n, functionToIntegrate, &all, lower, checkInfinity(upper)));
            }



			cout << "integral result: " << result << endl;
		}

		return result;
big	}



    double ProbabilityCalculator::functionToIntegrate(double x, void* data){


   	    allDims all = *((allDims *)data);
   	    all.integrals[all.current_index].value = correctValue(x);
        singleDim current;

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

            if (all.upperBoundThresholds.size() > 0) {

                double upperThreshold = all.upperBoundThresholds[index_next][0];

                for (int i = 0; i < index_next; i++) {
                    current = all.integrals[i];

                    double lowerFactor = 0.0;
                    double upperFactor = 0.0;
                    double upperThresholdFactor = 0.0;
                    if (all.lowerBounds[index_next].size() > i + 1)
                        lowerFactor = all.lowerBounds[index_next][i + 1];
                    if (all.lowerBounds[index_next].size() > i + 1)
                        upperFactor = all.upperBounds[index_next][i + 1];
                    if (all.upperBoundThresholds[index_next].size() > i + 1)
                        upperThresholdFactor = all.upperBoundThresholds[index_next][i + 1];

                    if (!isnan(lowerFactor) && lowerFactor != 0.0)
                        lower += (correctValue(lowerFactor) * correctValue(current.value));

                    if (!isnan(upperFactor) && upperFactor != 0.0)
                        upper += (correctValue(upperFactor) * correctValue(current.value));

                    if (!isnan(upperThresholdFactor) && upperThresholdFactor != 0.0)
                        upperThreshold += (correctValue(upperThresholdFactor) * correctValue(current.value));

                }

                if (all.integrals[all.current_index].fired && upper > upperThreshold)
                    upper = upperThreshold;

            } else {

                 for (int i = 0; i < index_next; i++) {
                    current = all.integrals[i];

                    double lowerFactor = 0.0;
                    double upperFactor = 0.0;

                    if (all.lowerBounds[index_next].size() > i + 1)
                        lowerFactor = all.lowerBounds[index_next][i + 1];
                     if (all.lowerBounds[index_next].size() > i + 1)
                        upperFactor = all.upperBounds[index_next][i + 1];

                    if (!isnan(lowerFactor) && lowerFactor != 0.0)
                        lower += (correctValue(lowerFactor) * correctValue(current.value));

                    if (!isnan(upperFactor)  && lowerFactor != 0.0)
                        upper += (correctValue(upperFactor) * correctValue(current.value));

                }
            }

            if ((lower > 0.0 && isinf(lower)) || upper <= 0.0)
                return 0.0;

   			return gauss_legendre(n, functionToIntegrate, &all, lower, checkInfinity(upper));
   		}

   	}



    double ProbabilityCalculator::checkInfinity(double value){

   		if (isinf(value))
   			return DBL_MAX;

   		return value;
   	}



   	double ProbabilityCalculator::correctValue(double value){

        if (value <= 0.0)
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