#include "ProbabilityCalculator.h"

using namespace std;
namespace hpnmg {

//#ifndef FABS
//	#define FABS(a) ((a)>=0?(a):-(a))
//#endif

	ProbabilityCalculator::ProbabilityCalculator(){}



	//todo: pass a pointer on a function instead, so that normal distribution has not to be defined every time
	double ProbabilityCalculator::normalPdf(double x, void* data)
	{
		double *p = (double *)data;
		double mu = *p;
		double sigma = *(++p);

		normal s(mu, sigma);
		return  pdf(s, x) ;
	}


	double ProbabilityCalculator::getProbability(){

	    /* numerical approximation of integral */
	    	double approx = -1.0;

	    	//double exact = 50.0;

	    	/* approximation error */
	    	double error;

	    	int i;

	    	try	{

				double parameters[2] ;
				parameters[0] = 5.0; //mu
				parameters[1] = 1.0; //sigma

				approx = gauss_legendre(1024,normalPdf,&parameters,0.0,8.0);
				//error = approx - exact;
				cout << "I = " << approx << ", n = " << 1024 << endl; //<<": error = " << FABS(error)


			} catch (const std::exception& e) {
				cout << "\n""Message from thrown exception was:\n" << e.what() << std::endl;
			}

	    	return approx;


	      return 0;

	}


}
