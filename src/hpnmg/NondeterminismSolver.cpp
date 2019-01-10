#include "NondeterminismSolver.h"

using namespace std;
using namespace alglib;

namespace hpnmg {


    NondeterminismSolver::NondeterminismSolver() {}


    //TODO diese Funktion muss später durch Janniks Model Checking sinnvoll ersetzt werden
    bool NondeterminismSolver::fulfillsProperty(ParametricLocationTree::Node node){

    std::vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
    if (discreteMarking[5] > 0)
        return true;

    return false;
}


    vector<ParametricLocationTree::Node> NondeterminismSolver::determineConflictSet(std::vector<ParametricLocationTree::Node> &children){

        vector<ParametricLocationTree::Node> nodes;
        vector<ParametricLocationTree::Node> others;

        for (ParametricLocationTree::Node child : children){
            EventType type = child.getParametricLocation().getSourceEvent().getEventType();

            if (type == Immediate || type == Timed)
                nodes.push_back(child);
            else
                others.push_back(child);
        }


        if (nodes.size() == 1) {
            others.push_back(nodes[0]);
            nodes.clear();
        }


        children = others;
        return nodes;
    }



    bool NondeterminismSolver::isCandidate(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates){

        for (ParametricLocationTree::Node currentCandidate : candidates){

            if (currentCandidate.getNodeID() == node.getNodeID() && fulfillsProperty(currentCandidate)){
                node = currentCandidate;
                return true;
            }
        }

        return false;
    }


    void NondeterminismSolver::recursivelyGetCandidates(vector<ParametricLocationTree::Node> &list, ParametricLocationTree::Node node, std::vector<ParametricLocationTree::Node> candidates){

        if (isCandidate(node, candidates))
            list.push_back(node);

        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(node);

        for (ParametricLocationTree::Node child : children)
            recursivelyGetCandidates(list, child, candidates);
    }



    double NondeterminismSolver::recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum){


        double maxOrMinProbability = 0.0;
        double currentProbability;

        //TODO Error beruecksichtigen
        double error = 0.0;

        int currentChildId;


        auto calculator = new ProbabilityCalculator();


        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
        vector<ParametricLocationTree::Node> conflictSet = NondeterminismSolver::determineConflictSet(children);

        if (conflictSet.size() > 0) {

            bool first = true;
            vector<int> currentBestChildIds;

            for (ParametricLocationTree::Node child: conflictSet) {

                currentChildId = child.getNodeID();
                currentProbability = recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum);

                if (first || (currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {

                    currentBestChildIds.clear();
                    currentBestChildIds.push_back(currentChildId);
                    maxOrMinProbability = currentProbability;
                    first = false;

                } else if (currentProbability == maxOrMinProbability) {
                    currentBestChildIds.push_back(currentChildId);
                }
            }

            (this->bestChildIds).push_back(currentBestChildIds);
        }



        for (ParametricLocationTree::Node child: children)
            maxOrMinProbability += recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum);


        if (isCandidate(currentNode, candidates)){

            if (algorithm == 0)
                //Gauss Legendre
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
            else
                //Monte Carlo
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, functioncalls, error);

        }
        return maxOrMinProbability;
    }





    void optimizationFunction(const real_1d_array &x, real_1d_array &fi, void* ptr)
    {
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

//TODO Hard gecodete Variablen richtig übergeben bzw. auslesen
int numberOfVariables = 2*2;
char algorithm = 3;
int functioncalls=1000;//50000
int evaluations = 128;
double error = 0.0;



    double prob = 0;
    int counter = 0;
    vector<double> conditions;

    nondetParams* params = (nondetParams *)ptr;


    //for all conflict locations
    for (std::vector<ParametricLocationTree::Node> nodes : (*params).orderedCandidates) {

       //for all candidates in sub-tree
       for (ParametricLocationTree::Node node : nodes) {

           ParametricLocation location = node.getParametricLocation();
           std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;

           //TODO multiple interval sets
           //for (std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> currentIntegrationIntervals : location.getIntegrationIntervals()) {
           std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> currentIntegrationIntervals = location.getIntegrationIntervals()[0];

           //for all random variables
           //TODO only for fired RV with corresponding continuous place -> how to find out?
           int i = 0;
           //for (int i = 0; i < currentIntegrationIntervals.size(); i++) {

                   conditions.push_back(currentIntegrationIntervals[i].second.first[0]);
                   fi[counter + 2] =conditions[counter] - x[counter]; //inequality <=0
                   currentIntegrationIntervals[i].second.first[0] = x[counter];
                   counter++;

                   conditions.push_back(currentIntegrationIntervals[i].second.second[0]);
                   fi[counter + 2] = x[counter] - conditions[counter]; //inequality <=0
                   currentIntegrationIntervals[i].second.second[0] = x[counter];
                   counter++;

                   //TODO müssen Abhängigkeiten von anderen Variablen hier auf Null gesetzt werden? Beim Prüfen der Constraints zu berücksichtigen!
//               }
               integrationIntervals.push_back(currentIntegrationIntervals);
//           }
           location.overwriteIntegrationIntervals(integrationIntervals);




        //map integration intervals to variables

//        auto calculator = new ProbabilityCalculator();
//        if (algorithm == 0)
//            //Gauss Legendre
//            prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(location, (*params).distributions, (*params).maxTime, evaluations);
//        else
//            //Monte Carlo
//            prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(location, (*params).distributions, (*params).maxTime, algorithm, functioncalls, error);
//
//

//TEST



        }



    }

    fi[0] = (-1.0)*((x[1]-x[0]) + (x[3]-x[2]));



    //Constraints

    //x[0] <= x[1]
    fi[counter + 2] = x[0] - x[1]; //inequality <=0
    counter++;

    //x[2] <= x[3]
    fi[counter + 2] = x[2] - x[3]; //inequality <=0
    counter++;

    //x[1] = x[2]  OR x[0] = x[3]
   fi[1] = (x[1] - x[2])*(x[0] - x[3]); //equality == 0
//fi[1] = 0.0;


}


    double NondeterminismSolver::recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum){




           double maxOrMinProbability = 0.0;
           double currentProbability;

           //TODO Error beruecksichtigen
           double error = 0.0;

           int currentChildId;

           auto calculator = new ProbabilityCalculator();

           vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
           vector<ParametricLocationTree::Node> conflictSet = NondeterminismSolver::determineConflictSet(children);
           void *ptr;
           nondetParams params;

           if (conflictSet.size() > 0) {
            for (ParametricLocationTree::Node conflictNode : conflictSet){

                vector<ParametricLocationTree::Node> currentCandidates;
                recursivelyGetCandidates(currentCandidates, conflictNode,  candidates);
                params.orderedCandidates.push_back(currentCandidates);
            }


            params.distributions = (*this->plt).getDistributions();
            params.maxTime = (*this->plt).getMaxTime();



           real_1d_array x0 = "[0,0,0,0]";
           real_1d_array s = "[1,1,1,1]";
           double epsx = 00000.1;//0.000001;
           ae_int_t maxits = 0;//0; for unlimited
           minnlcstate state; //state object
           minnlcreport rep;
           real_1d_array x1;

           //
           // This variable contains differentiation step
           //
           double diffstep = 1.0e-6;


           // Create optimizer object, choose AUL algorithm and tune its settings:
           // * rho=1000       penalty coefficient
           // * outerits=5     number of outer iterations to tune Lagrange coefficients
           // * epsx=0.000001  stopping condition for inner iterations
           // * s=[1,1]        all variables have unit scale
           // * exact low-rank preconditioner is used, updated after each 10 iterations
           // * upper limit on step length is specified (to avoid probing locations where exp() is large)
           //
           minnlccreatef(4, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
           minnlcsetalgoslp(state); //activate
           minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
           minnlcsetscale(state, s); //set scale (here always 1)
           minnlcsetstpmax(state, 10.0); //settings

               //
               // Set constraints:
               //
               // Nonlinear constraints are tricky - you can not "pack" general
               // nonlinear function into double precision array. That's why
               // minnlcsetnlc() does not accept constraints itself - only constraint
               // counts are passed: first parameter is number of equality constraints,
               // second one is number of inequality constraints.
               //
               //
               // NOTE: MinNLC optimizer supports arbitrary combination of boundary, general
               //       linear and general nonlinear constraints. This example does not
               //       show how to work with boundary or general linear constraints, but you
               //       can easily find it in documentation on minnlcsetbc() and
               //       minnlcsetlc() functions.
               //
               minnlcsetnlc(state, 1, 6);

               //
               // Optimize and test results.    //

               //

               alglib::minnlcoptimize(state, optimizationFunction, NULL, &params); //start optimizer
               minnlcresults(state, x1, rep); //get results
               printf("%s\n", x1.tostring(5).c_str()); // EXPECTED: [-0.70710,-0.70710,0.49306]
               //return 0;












//               bool first = true;
//               vector<int> currentBestChildIds;
//
//               for (ParametricLocationTree::Node child: conflictSet) {
//
//                   currentChildId = child.getNodeID();
//                   currentProbability = recursivelySolveNondeterminismProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum);
//
//                   if (first || (currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {
//
//                       currentBestChildIds.clear();
//                       currentBestChildIds.push_back(currentChildId);
//                       maxOrMinProbability = currentProbability;
//                       first = false;
//
//                   } else if (currentProbability == maxOrMinProbability) {
//                       currentBestChildIds.push_back(currentChildId);
//                   }
//               }
//
//               (this->bestChildIds).push_back(currentBestChildIds);
           }



           for (ParametricLocationTree::Node child: children)
               maxOrMinProbability += recursivelySolveNondeterminismProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum);


           if (isCandidate(currentNode, candidates)){

               if (algorithm == 0)
                   //Gauss Legendre
                   maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
               else
                   //Monte Carlo
                   maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, functioncalls, error);

           }
           return maxOrMinProbability;
       }




    double NondeterminismSolver::solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic){

        this->plt = plt;

        if (prophetic)
            return recursivelySolveNondeterminismProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum);
        else
            return recursivelySolveNondeterminismNonProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum);
    }


    vector<vector<int>> NondeterminismSolver::getBestChildIds(){
        return this->bestChildIds;
    }

}
