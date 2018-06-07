#include "ProbabilityCalculator.h"

using namespace std;

namespace hpnmg {

//#ifndef FABS
//	#define FABS(a) ((a)>=0?(a):-(a))
//#endif

	ProbabilityCalculator::ProbabilityCalculator(){}

//
//	double ProbabilityCalculator::normalPdf(double x, void* data){
//
//		normal *myn = (normal *)data;
//		return  pdf(*myn, x) ;
//	}


	typedef struct {
		double value;
		//double left;
		//double right;
		normal dist;
	} singleDim;



	typedef struct {
		int current_index;
		vector<singleDim> integrals;
		vector<vector<double>> lowerBounds;
		vector<vector<double>> upperBounds;
	} allDims;



	double ProbabilityCalculator::f(double x, void* data){

		//if (inner which is most dependent) = integrand f(x,y,z) = product of all distributions at fixed x and y
		//	return joint distribution integral
		//else
		// return recursive call of gaus_legrendre



		allDims all = *((allDims *)data);
		singleDim current;

		all.integrals[all.current_index].value = x;

		if (all.current_index  == (all.integrals.size() - 1)){

			double result = 1.0;
			for (int i = 0; i < all.integrals.size(); i++){
				current = all.integrals[all.current_index];
				result *= pdf(current.dist, current.value);
			}

			return result;

		} else {


			all.current_index++;
			int index = all.current_index;
			//current = all.integrals[index];


		//	for (int i = 1; i < all.integrals.size(); i++)

			return gauss_legendre(n, f, &all, all.lowerBounds[index][0], all.upperBounds[index][0]);
			//return 1.0;
		}

	}


	double ProbabilityCalculator::getProbability(const vector<ParametricLocationTree::Node> &nodes, const ParametricLocationTree &tree){

		double total = 0.0;

		cout << "nodes size = " << nodes.size() << endl;

		for (int i = 0;i < nodes.size(); i++){
			cout << i << endl;
			total += nodes[i].getParametricLocation().getConflictProbability() * calculateIntervals(nodes[i].getParametricLocation(), tree);
		}

		return total;

	}





	double ProbabilityCalculator::calculateIntervals(const ParametricLocation &location, const ParametricLocationTree &tree){


		double result = -1.0;

		allDims all;
		all.current_index = 0;


//			//todo
//			double mu1 = 0.0;
//			double sigma1 = 1.0;
//			normal dist1 (mu1, sigma1);//
//
//			allDims all;
//			all.current_index = 0;

//
//			vector<double> dependencies = location.getSourceEvent().getGeneralDependencies();
//			vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
//
//			cout << "dim = " << location.getDimension() << ", size = " << generalTransitionsFired.size() << endl;
//
//			vector<int> counter = vector<int>(location.getDimension() - 1);
//			fill(counter.begin(), counter.end(),0);


//			for (int &firedTransition : generalTransitionsFired){
//
//
//				int transitionID = generalTransitionsFired[firedTransition];
//				int firing = counter[transitionID];
//				counter[transitionID] -= 1;
//
//				counter[transitionID] +=1;
//			}


//			while (dependencies.size() > 0) {
//
//				dependencies.pop_back();
//
//			    int transitionID = generalTransitionsFired[dependencies.size()];
//			    int firingTime = counter[transitionID];
//				counter[transitionID] -= 1;
//
//				singleDim s;
//				s.lowerBound = location.getGeneralIntervalBoundLeft()[transitionID][firingTime - 1];
//				s.upperBound = location.getGeneralIntervalBoundRight()[transitionID][firingTime - 1];
//				s.dist = dist1;
//				all.integrals.push_back(s);
//
//			}





		int dimension = location.getDimension();


		double mu1 = 0.0;
		double sigma1 = 1.0;
		normal dist1 (mu1, sigma1);
		singleDim s1;
		vector<double> lower1(dimension, 0);
		vector<double> upper1(dimension, 0);
		all.lowerBounds.push_back(lower1);
		all.upperBounds.push_back(upper1);
		all.lowerBounds[0][0] = 0.0;
		all.upperBounds[0][0] = 2.0;
		s1.dist = dist1;
		all.integrals.push_back(s1);

		double mu2 = 0.0;
		double sigma2 = 1.0;
		normal dist2 (mu2, sigma2);
		singleDim s2;
		vector<double> lower2(dimension, 0);
		vector<double> upper2(dimension, 0);
		all.lowerBounds.push_back(lower2);
		all.upperBounds.push_back(upper2);
		all.lowerBounds[1][0] = 0.0;
		all.lowerBounds[1][0] = 0.5;
		s2.dist = dist2;
		all.integrals.push_back(s2);

		double mu3 = 0.0;
		double sigma3 = 1.0;
		normal dist3 (mu3, sigma3);
		singleDim s3;
		vector<double> lower3(dimension, 0);
		vector<double> upper3(dimension, 0);
		all.lowerBounds.push_back(lower3);
		all.upperBounds.push_back(upper3);
		all.lowerBounds[2][0] = 0.1;
		all.lowerBounds[2][0] = 0.7;
		s3.dist = dist3;
		all.integrals.push_back(s3);



		try	{

			//result = gauss_legendre(n, f, &all, all.integrals[0].lowerBound[0], all.integrals[0].upperBound[0]);
			result = gauss_legendre(n, f, &all, all.lowerBounds[0][0], all.upperBounds[0][0]);

			std::cout.precision(17);
			cout << "I = " << result << ", n = " << n << endl;


		} catch (const std::exception& e) {
			cout << "\n""Message from thrown exception was:\n" << e.what() << std::endl;
		}

		return result;
	}




//
//	double ParametricLocation::getMinimumTime(vector<vector<vector<double>>> lowerBoundaries, vector<vector<vector<double>>> upperBoundaries) {
//	   //     double time = sourceEvent.getTime();
//
//			vector<double> dependencies = sourceEvent.getGeneralDependencies();
//	        vector<int> counter = vector<int>(dimension -1);
//	        fill(counter.begin(), counter.end(),0);
//	        for (int &firedTransition : generalTransitionFired)
//	            counter[firedTransition] +=1;
//
//
//	        while (dependencies.size() > 0) {
//	            double value = dependencies.back();
//	            dependencies.pop_back();
//
//	            int transitionPosition = generalTransitionFired[dependencies.size()];
//	            int firingTime = counter[transitionPosition];
//	            vector<double> lowerBounds = lowerBoundaries[transitionPosition][firingTime - 1];
//	            vector<double> upperBounds = upperBoundaries[transitionPosition][firingTime - 1];
//
//	            counter[transitionPosition] -= 1;
//
//	            if (value >= 0) {
//	                time += value * lowerBounds[0];
//	            } else {
//	                time += value * upperBounds[0];
//	            }
//
//	            for (int i = 1; i <= dependencies.size(); ++i) {
//	                if (value >= 0) {
//	                    dependencies[i] += value * lowerBounds[i];
//	                } else {
//	                    dependencies[i] += value * upperBounds[i];
//	                }
//	            }
//	        }
//
//	        return time;
//	    }


}
