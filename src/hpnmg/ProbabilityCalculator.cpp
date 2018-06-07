#include "ProbabilityCalculator.h"

using namespace std;

namespace hpnmg {


ProbabilityCalculator::ProbabilityCalculator(){}


	typedef struct {
		double value;
		normal dist;
	} singleDim;



	typedef struct {
		int current_index;
		vector<singleDim> integrals;
		vector<vector<double>> lowerBounds;
		vector<vector<double>> upperBounds;
	} allDims;



	double ProbabilityCalculator::f(double x, void* data){


		singleDim current;

		allDims all = *((allDims *)data);
		all.integrals[all.current_index].value = x;


		if (all.current_index  == (all.integrals.size() - 1)){

			double result = 1.0;
			for (int i = 0; i < all.integrals.size(); i++){
				current = all.integrals[i];
				result *= pdf(current.dist, current.value);
			}

			return result;

		} else {

			all.current_index++;
			int index_next = all.current_index;

			double lower = all.lowerBounds[index_next][0];
			double upper = all.upperBounds[index_next][0];

			for (int i = 0; i < all.integrals.size() - 1; i++){
				current = all.integrals[i];
				lower += all.lowerBounds[index_next][i+1] * current.value;
				upper += all.upperBounds[index_next][i+1] * current.value;
			}

			return gauss_legendre(n, f, &all, lower, upper);
		}

	}



	double ProbabilityCalculator::getProbability(const vector<ParametricLocationTree::Node> &nodes, const ParametricLocationTree &tree){

		double total = 0.0;

		for (int i = 0;i < nodes.size(); i++)
			total +=  nodes[i].getParametricLocation().getConflictProbability() *calculateIntervals(nodes[i].getParametricLocation(), tree);


		std::cout.precision(17);
		cout << "nodes size = " << nodes.size() << endl;
		cout << "total = " << total << ", n = " << n << endl;

		return total;

	}



	double ProbabilityCalculator::calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree){


		double result = 1.0;

		allDims all;
		all.current_index = 0;
		int dimension = location.getDimension();

		//todo distributions
		double mu1 = 5.0;
		double sigma1 = 3.0;
		normal dist1 (mu1, sigma1);//

		vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();

		vector<int> counter = vector<int>(location.getDimension() - 1);
		fill(counter.begin(), counter.end(),0);

		for (int i = 0;i < generalTransitionsFired.size(); i++){

			int transitionID = generalTransitionsFired[i];
			int firing = counter[transitionID];

			singleDim s;
			s.dist = dist1;
			all.integrals.push_back(s);

			all.lowerBounds.push_back(location.getGeneralIntervalBoundLeft()[transitionID][firing]);
			all.upperBounds.push_back(location.getGeneralIntervalBoundRight()[transitionID][firing]);

			counter[transitionID] +=1;
		}


		//todo nicht gefeuerte Zufallsvariablen fehlen noch! vermutlich: vergleich von counter mit getGeneralIntervalBoundLeft()[transitionID].size - Frage: können die nicht gefeuerten ZV gegenseitig abhängig sein?


		if (generalTransitionsFired.size() > 0)
			result = gauss_legendre(n, f, &all, all.lowerBounds[0][0], all.upperBounds[0][0]);

		cout << "integral result: " << result << endl;

		return result;
	}
}
