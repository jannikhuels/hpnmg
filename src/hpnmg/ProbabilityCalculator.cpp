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



	double ProbabilityCalculator::getDensity(pair<string, map<string, float>> distribution, double value){

		map<string, float> normalMap = distribution.second;

        if (isinf(value) || isnan(value))
            return 0.0;

		try{
			if (distribution.first == "normal"){

                cout << "Normal distribution not supported due to negative values. Use folded normal distribution instead." << endl;
                throw std::invalid_argument( "invalid distribution" );


            } else  if (distribution.first == "foldednormal"){

                boost::math::normal dist ((double) normalMap.at("mu"), (double)normalMap.at("sigma"));
                return (boost::math::pdf(dist, value) + boost::math::pdf(dist, (-1.0)*value));


            } else  if (distribution.first == "halfnormal"){

                boost::math::normal dist (0.0, (double)normalMap.at("sigma"));
                return (boost::math::pdf(dist, value) + boost::math::pdf(dist, (-1.0)*value));


            } else  if (distribution.first == "lognormal"){

                boost::math::lognormal dist ((double) normalMap.at("mu"), (double)normalMap.at("sigma"));
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "inversenormal"){

                boost::math::inverse_gaussian dist ((double) normalMap.at("mu"), (double)normalMap.at("lambda"));
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "beta"){

                boost::math::beta_distribution<double> dist ((double) normalMap.at("alpha"), (double)normalMap.at("beta"));
                return boost::math::pdf(dist, value);


            } else  if (distribution.first == "exp"){

                boost::math::lognormal dist ((double) normalMap.at("lambda"));
                return boost::math::pdf(dist, value);


			} else  if (distribution.first == "uniform"){

                double a = (double) normalMap.at("a");
                double b = (double) normalMap.at("b");

                if (b <= a || a < 0.0) {
                    cout << "Invalid uniform distribution. Make sure that 0 >= a > b." << endl;
                    throw std::invalid_argument("invalid distribution");
                }

                if (value < a || value > b)
                    return 0.0;

                boost::math::uniform dist (a,b);
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



	double ProbabilityCalculator::checkInfinity(double value){

		if (isinf(value))
			return DBL_MAX;

		return value;
	}


    double ProbabilityCalculator::correctValue(double value){

			if (value < pow(10, -100))
            return 0.0;

        return value;
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

			//TODO wenn enabled, dann Zeitdifferenz draufrechnen
//			if (enabledTransitions[j] == true){
//
//				all.lowerBounds[all.integrals.size() - 1][0] += location.getSourceEvent().getTime();
//
//				vector<double> dependencies = location.getSourceEvent().getGeneralDependencies();
//				for (int k = 1; k < dependencies.size(); k++)
//					all.lowerBounds[all.integrals.size() - 1][k] += dependencies[k];
//
///			}

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





    double ProbabilityCalculator::getProbability(const vector<ParametricLocationTree::Node> &nodes, const ParametricLocationTree &tree, double timepoint){

        double total = 0.0;


        //TODO womit multiplizieren?
        for (int i = 0;i < nodes.size(); i++)
            total +=  calculateIntervals(nodes[i].getParametricLocation(), tree, timepoint); // * nodes[i].getParametricLocation().getCummulativeProbability();


        std::cout.precision(17);
        cout << "nodes size = " << nodes.size() << endl;
        cout << "total = " << total << ", n = " << n << endl;
        std::cout.precision(6);

        return total;

    }

}
