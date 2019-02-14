#include "NondeterminismSolver.h"

using namespace std;
using namespace alglib;

namespace hpnmg {


    NondeterminismSolver::NondeterminismSolver() {}


    //TODO diese Funktion muss später durch Janniks / Henners Model Checking sinnvoll ersetzt werden, vermutlich werden dann noch Intervalle eingeschränkt
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



    bool NondeterminismSolver::isCandidateForProperty(ParametricLocationTree::Node &node,
                                                      std::vector<ParametricLocationTree::Node> candidates){

        for (ParametricLocationTree::Node currentCandidate : candidates){

            if (currentCandidate.getNodeID() == node.getNodeID() && fulfillsProperty(currentCandidate)){
                node = currentCandidate;
                return true;
            }
        }

        return false;
    }


    void NondeterminismSolver::recursivelyGetCandidates(vector<ParametricLocationTree::Node> &list, ParametricLocationTree::Node node, std::vector<ParametricLocationTree::Node> candidates){

        if (isCandidateForProperty(node, candidates))
            list.push_back(node);

        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(node);

        for (ParametricLocationTree::Node child : children)
            recursivelyGetCandidates(list, child, candidates);
    }



    double NondeterminismSolver::recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error){


        double maxOrMinProbability = 0.0;
        double currentProbability;
        double currentError;

        int currentChildId;


        auto calculator = new ProbabilityCalculator();


        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
        vector<ParametricLocationTree::Node> conflictSet = NondeterminismSolver::determineConflictSet(children);

        if (conflictSet.size() > 0) {

            bool first = true;
            vector<int> currentBestChildIds;

            for (ParametricLocationTree::Node child: conflictSet) {

                currentChildId = child.getNodeID();
                currentError = 0.0;
                currentProbability = recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError);

//TODO error in vergleich mit einbeziehen?
                if (first || (currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {

                    currentBestChildIds.clear();
                    currentBestChildIds.push_back(currentChildId);
                    maxOrMinProbability = currentProbability;
                    error = currentError;
                    first = false;

//TODO error in vergleich mit einbeziehen?
                } else if (currentProbability == maxOrMinProbability) {
                    currentBestChildIds.push_back(currentChildId);
                    if (currentError > error)
                        error = currentError;
                }
            }

            (this->bestChildIds).push_back(currentBestChildIds);
        }



        for (ParametricLocationTree::Node child: children){
            maxOrMinProbability += recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError);
            error += currentError;
        }


        if (isCandidateForProperty(currentNode, candidates)){

            currentError = 0.0;

            if (algorithm == 0)
                //Gauss Legendre
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
            else
                //Monte Carlo
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, functioncalls, currentError);

            error += currentError;
        }

        return maxOrMinProbability;
    }




//TODO error berechnen
    void optimizationFunction(const real_1d_array &x, real_1d_array &fi, void* ptr){

        double error = 0.0;
        double prob = 0.0;
        int counterConstraints = 1;

        double a = x[0];
        //cout << a << endl;
        nondetParams* params = (nondetParams *)ptr;

        //for all random variables
        //TODO only for fired RV with corresponding continuous place -> how to find out? for now only first variable
        int i = 0;
        bool found;


//START First Sub-tree (möglicherweise in eigene Funktion auslagern?)

        vector<ParametricLocationTree::Node> nodes = (*params).candidatesLeft;
        //nodes = (*params).candidatesRight;


        //for all candidates in sub-tree
        for (ParametricLocationTree::Node node : nodes) {

            ParametricLocation location = node.getParametricLocation();
            std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;

            std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> currentIntegrationIntervals;
            found = false;

            //for multiple interval sets, find interval which contains a
            vector<vector<pair<int, pair<vector<double>, vector<double>>>>> intervals = location.getIntegrationIntervals();
            for (int j = 0; j < intervals.size();j++){
                if (intervals[j][i].second.first[0] <= x[0] && intervals[j][i].second.second[0] >= x[0]) {
                    currentIntegrationIntervals = intervals[j];
                    found = true;
                    break;
                }
            }


            if (found){

                fi[counterConstraints] = currentIntegrationIntervals[i].second.first[0] - a; //inequality <=0
                counterConstraints++;
                fi[counterConstraints] = a - currentIntegrationIntervals[i].second.second[0]; //inequality <=0
                counterConstraints++;
                currentIntegrationIntervals[i].second.second[0] = a;
                //currentIntegrationIntervals[i].second.first[0] = a;

                //TODO Sonderfall wenn Abhängigkeiten von anderen RV bestehen

                integrationIntervals.push_back(currentIntegrationIntervals);
                location.overwriteIntegrationIntervals(integrationIntervals);


                auto calculator = new ProbabilityCalculator();
                if ((*params).algorithm == 0)
                    //Gauss Legendre
                    prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(location, (*params).distributions, (*params).maxTime, (*params).evaluations);
                else
                    //Monte Carlo
                    prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(location, (*params).distributions, (*params).maxTime, (*params).algorithm, (*params).functioncalls, error);

            }
        }


//ENDE First Sub-tree

//START Second Sub-tree


        nodes = (*params).candidatesRight;

        //for all candidates in sub-tree
        for (ParametricLocationTree::Node node : nodes) {

            ParametricLocation location = node.getParametricLocation();
            std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;

            std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> currentIntegrationIntervals;
            found = false;

            //for multiple interval sets, find interval which contains a
            for (int j = 0; j < location.getIntegrationIntervals().size();j++){
                if (location.getIntegrationIntervals()[j][i].second.first[0] <= a && location.getIntegrationIntervals()[j][i].second.second[0] >= a) {
                    currentIntegrationIntervals = location.getIntegrationIntervals()[j];
                    found = true;
                    break;
                }
            }


            if (found){

                fi[counterConstraints] = currentIntegrationIntervals[i].second.first[0] - a; //inequality <=0
                counterConstraints++;
                fi[counterConstraints] = a - currentIntegrationIntervals[i].second.second[0]; //inequality <=0
                counterConstraints++;
                currentIntegrationIntervals[i].second.first[0] = a;

                //TODO Sonderfall wenn Abhängigkeiten von anderen RV bestehen

                integrationIntervals.push_back(currentIntegrationIntervals);
                location.overwriteIntegrationIntervals(integrationIntervals);


                auto calculator = new ProbabilityCalculator();
                if ((*params).algorithm == 0)
                    //Gauss Legendre
                    prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(location, (*params).distributions, (*params).maxTime, (*params).evaluations);
                else
                    //Monte Carlo
                    prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(location, (*params).distributions, (*params).maxTime, (*params).algorithm, (*params).functioncalls, error);

            }
        }

//ENDE Second Sub-tree



        fi[0] = (-1.0) * prob;

       // if (fi[1] <= 0 && fi[2] <= 0  && fi[3] <= 0 && fi[4] <= 0)
       //     cout << " TOTAL: " << a  << " --- Prob: " <<  prob << endl;


        (*params).prob = prob;
        (*params).result = a;


    }


    double NondeterminismSolver::recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error){


       double maxOrMinProbability = 0.0;
       double currentProbability;
       double currentError;

       auto calculator = new ProbabilityCalculator();

       vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
       vector<ParametricLocationTree::Node> conflictSet = NondeterminismSolver::determineConflictSet(children);


       if (conflictSet.size() > 0) {

            //TODO extend for more than two conflict sub-trees
           vector<nondetParams> params;


           real_1d_array x0 = "[0]";
           real_1d_array s = "[1]";
           double epsx = 0.01;//0.000001;
           ae_int_t maxits = 10;//0; for unlimited
           minnlcstate state; //state object
           minnlcreport rep;
           real_1d_array x1;
           double diffstep = 1.0;



            for (int i = 0; i <= 1; i++){

               nondetParams currentParams;
               currentParams.algorithm = algorithm;
               currentParams.evaluations = min(64, evaluations);
               currentParams.functioncalls = min(1000, functioncalls);
               currentParams.distributions = (*this->plt).getDistributions();
               currentParams.maxTime = (*this->plt).getMaxTime();


               vector<ParametricLocationTree::Node> currentCandidates0;
               recursivelyGetCandidates(currentCandidates0, conflictSet[i%2], candidates);
               currentParams.candidatesRight = currentCandidates0;

               vector<ParametricLocationTree::Node> currentCandidates1;
               recursivelyGetCandidates(currentCandidates1, conflictSet[(i+1)%2], candidates);
               currentParams.candidatesLeft = currentCandidates1;

               minnlccreatef(1, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
               minnlcsetalgoslp(state); //activate
               minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
               minnlcsetscale(state, s); //set scale (here always 1)
               minnlcsetstpmax(state, 0.0); //settings
               minnlcsetnlc(state, 0, 4);
               minnlcoptimize(state, optimizationFunction, NULL, &currentParams); //start optimizer
               minnlcresults(state, x1, rep); //get results

               if (rep.terminationtype == 2)
                   cout << "Pre-computation " << i+1 << " succeeded: x = " << currentParams.result << " with P_max = " << currentParams.prob << endl;
               else
                   cout << "Pre-computation " << i+1 << " failed" << endl;

               params.push_back(currentParams);

           }

            int i;
            if(params[0].prob >= params[1].prob)
                i = 0;
            else
                i = 1;
            params[i].functioncalls = functioncalls;
            params[i].evaluations = evaluations;
            minnlccreatef(1, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
            minnlcsetalgoslp(state); //activate
            minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
            minnlcsetscale(state, s); //set scale (here always 1)
            minnlcsetstpmax(state, 0.0); //settings
            minnlcsetnlc(state, 0, 4);
            minnlcoptimize(state, optimizationFunction, NULL, &params[i]); //start optimizer
            minnlcresults(state, x1, rep); //get results

           if (rep.terminationtype == 2)
               cout << "Main computation succeeded: x = " << params[i].result << " with P_max = " << params[i].prob << endl;
           else
               cout << "Main computation failed" << endl;


           maxOrMinProbability = params[i].prob;
           //TODO Error

       }



        for (ParametricLocationTree::Node child: children){
            maxOrMinProbability += recursivelySolveNondeterminismProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError);
            error += currentError;
        }



        if (isCandidateForProperty(currentNode, candidates)){

            currentError = 0.0;

            if (algorithm == 0)
                //Gauss Legendre
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
            else
                //Monte Carlo
                maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(currentNode.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, functioncalls, currentError);

            error += currentError;
        }


       return maxOrMinProbability;
   }




    double NondeterminismSolver::solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic, double &error){

        this->plt = plt;

        if (prophetic)
            return recursivelySolveNondeterminismProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum, error);
        else
            return recursivelySolveNondeterminismNonProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum, error);
    }


    vector<vector<int>> NondeterminismSolver::getBestChildIds(){
        return this->bestChildIds;
    }

}
