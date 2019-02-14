#include <iostream>
#include <ctime>
#include "gtest/gtest.h"
#include <chrono>
#include <cstdint>

#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include "datastructures/Event.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"
#include "PLTWriter.h"
#include "NondeterminismSolver.h"


#include "../src/hpnmg/optimization/stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../src/hpnmg/optimization/optimization.h"

using namespace hpnmg;
using namespace std;

using namespace alglib;






TEST(ParametricLocationTreeXML, Nondeterministic) {

    cout << endl;

    double maxTime = 6.0;
    double checkTime = maxTime;
//    double timeStart = maxTime;
//    double timeEnd = maxTime;
//    std::pair<double,double> interval;
//    interval.first = timeStart;
//    interval.second = timeEnd;


    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicConflicts/nondet2_2g.xml");

    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 2);

    //int dim = plt->getDimension();

    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);


//TODO sollte auch für time interval möglich sein
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(checkTime);


    NondeterminismSolver solver;
    double error;
    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), candidates, 3, 5000, 128, false, true, error);

    vector<vector<int>> bestIds = solver.getBestChildIds();

   // cout << "Max probability: " << maxprob << endl;
}




//
//void function1_func(const real_1d_array &x, double &func, void *ptr)
//{
//    //
//    // this callback calculates f(x0,x1) = 100*(x0+3)^4 + (x1-3)^4
//    //
//    func = 100*pow(x[0]+3,4) + pow(x[1]-3,4);
//}
//
//TEST(NonDeterministicChoices, linear) {
//    //
//    // This example demonstrates minimization of f(x,y) = 100*(x+3)^4+(y-3)^4
//    // subject to inequality constraints:
//    // * x>=2 (posed as general linear constraint),
//    // * x+y>=6
//    // using BLEIC optimizer.
//    //
//    real_1d_array x = "[5,5]";
//    real_2d_array c = "[[1,0,2],[1,1,6]]";
//    integer_1d_array ct = "[1,1]";
//    minbleicstate state;
//    minbleicreport rep;
//
//    //
//    // These variables define stopping conditions for the optimizer.
//    //
//    // We use very simple condition - |g|<=epsg
//    //
//    double epsg = 0.000001;
//    double epsf = 0;
//    double epsx = 0;
//    ae_int_t maxits = 0;
//
//    //
//    // This variable contains differentiation step
//    //
//    double diffstep = 1.0e-6;
//
//    //
//    // Now we are ready to actually optimize something:
//    // * first we create optimizer
//    // * we add linear constraints
//    // * we tune stopping conditions
//    // * and, finally, optimize and obtain results...
//    //
//    minbleiccreatef(x, diffstep, state);
//    minbleicsetlc(state, c, ct);
//    minbleicsetcond(state, epsg, epsf, epsx, maxits);
//    alglib::minbleicoptimize(state, function1_func);
//    minbleicresults(state, x, rep);
//
//    //
//    // ...and evaluate these results
//    //
//    printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4
//    printf("%s\n", x.tostring(2).c_str()); // EXPECTED: [2,4]
//}
//
//
//
//
//
//
//
//
//
//void  nlcfunc2(const real_1d_array &x, real_1d_array &fi, void *ptr)
//{
//    //
//    // this callback calculates
//    //
//    //     f0(x0,x1,x2) = x0+x1
//    //     f1(x0,x1,x2) = x2-exp(x0)
//    //     f2(x0,x1,x2) = x0^2+x1^2-1
//    //
//    fi[0] = x[0]+x[1];
//    fi[1] = x[2]-exp(x[0]);
//    fi[2] = x[0]*x[0] + x[1]*x[1] - 1.0;
//
//    int test = *((int *)ptr);
//
//}
//
//
//
//TEST(NonDeterministicChoices, nonlinear){
//
//    cout << "Starting test\n";
//
//
//    //
//    // This example demonstrates minimization of
//    //
//    //     f(x0,x1) = x0+x1
//    //
//    // subject to nonlinear inequality constraint
//    //
//    //    x0^2 + x1^2 - 1 <= 0
//    //
//    // and nonlinear equality constraint
//    //
//    //    x2-exp(x0) = 0
//    //
//    real_1d_array x0 = "[0,0,0]";
//    real_1d_array s = "[1,1,1]";
//    double epsx = 0.000001;
//    ae_int_t maxits = 0;
//    minnlcstate state; //state object
//    minnlcreport rep;
//    real_1d_array x1;
//
//    //
//    // This variable contains differentiation step
//    //
//    double diffstep = 1.0e-6;
//
//
//    //
//    // Create optimizer object, choose AUL algorithm and tune its settings:
//    // * rho=1000       penalty coefficient
//    // * outerits=5     number of outer iterations to tune Lagrange coefficients
//    // * epsx=0.000001  stopping condition for inner iterations
//    // * s=[1,1]        all variables have unit scale
//    // * exact low-rank preconditioner is used, updated after each 10 iterations
//    // * upper limit on step length is specified (to avoid probing locations where exp() is large)
//    //
//    minnlccreatef(3, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
//    minnlcsetalgoslp(state); //activate
//    minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
//    minnlcsetscale(state, s); //set scale (here always 1)
//    minnlcsetstpmax(state, 10.0); //settings
//
//    //
//    // Set constraints:
//    //
//    // Nonlinear constraints are tricky - you can not "pack" general
//    // nonlinear function into double precision array. That's why
//    // minnlcsetnlc() does not accept constraints itself - only constraint
//    // counts are passed: first parameter is number of equality constraints,
//    // second one is number of inequality constraints.
//    //
//    //
//    // NOTE: MinNLC optimizer supports arbitrary combination of boundary, general
//    //       linear and general nonlinear constraints. This example does not
//    //       show how to work with boundary or general linear constraints, but you
//    //       can easily find it in documentation on minnlcsetbc() and
//    //       minnlcsetlc() functions.
//    //
//    minnlcsetnlc(state, 1, 1);
//
//    //
//    // Optimize and test results.    //
//
//    //
//    void *ptr;
//    int test = 286;
//    ptr = &test;
//    alglib::minnlcoptimize(state, nlcfunc2, NULL, &test); //start optimizer
//    minnlcresults(state, x1, rep); //get results
//    printf("%s\n", x1.tostring(5).c_str()); // EXPECTED: [-0.70710,-0.70710,0.49306]
//    //return 0;
//
//
//
//}
