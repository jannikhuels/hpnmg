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
		double left;
		double right;
		normal dist;
	} singleDim;


	typedef struct {
		int current_index;
		vector<singleDim> integrals;
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
			current = all.integrals[all.current_index];

			return gauss_legendre(n, f, &all, current.left*x, current.right*x);
		}

	}


	double ProbabilityCalculator::getProbability(){



	    	double result = -1.0;

			allDims all;
			all.current_index = 0;

			double mu1 = 0.0;
			double sigma1 = 1.0;
			normal dist1 (mu1, sigma1);
			singleDim s1;
			s1.left = 0.0;
			s1.right = 2.0;
			s1.dist = dist1;
			all.integrals.push_back(s1);

			double mu2 = 0.0;
			double sigma2 = 1.0;
			normal dist2 (mu2, sigma2);
			singleDim s2;
			s2.left = 0.0;
			s2.right = 0.5;
			s2.dist = dist2;
			all.integrals.push_back(s2);

			double mu3 = 0.0;
			double sigma3 = 1.0;
			normal dist3 (mu3, sigma3);
			singleDim s3;
			s3.left = 0.1;
			s3.right = 0.7;
			s3.dist = dist3;
			all.integrals.push_back(s3);

		    try	{

		    	result = gauss_legendre(n, f, &all, s1.left, s1.right);

	    		std::cout.precision(17);
				cout << "I = " << result << ", n = " << n << endl;


			} catch (const std::exception& e) {
				cout << "\n""Message from thrown exception was:\n" << e.what() << std::endl;
			}

	    	return result;

	}


}
