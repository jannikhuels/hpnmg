#include "NondeterminismSolver.h"

using namespace std;
using namespace alglib;

namespace hpnmg {


    NondeterminismSolver::NondeterminismSolver() {}


    //TODO diese Funktion muss später durch Janniks / Henners Model Checking sinnvoll ersetzt werden, vermutlich werden dann noch Intervalle eingeschränkt
    bool NondeterminismSolver::fulfillsProperty(ParametricLocationTree::Node node){

    std::vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
    if (discreteMarking[5] > 0) //fig5: discreteMarking[5] > 0 , fig1: discreteMarking[4] > 0
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

                if (first || (currentProbability - currentError > maxOrMinProbability + error && minimum == false) || (currentProbability + currentError < maxOrMinProbability - error && minimum == true)) { //error intervals do not overlap
               // if (first || (currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {
                    currentBestChildIds.clear();
                    currentBestChildIds.push_back(currentChildId);
                    maxOrMinProbability = currentProbability;
                    error = currentError;
                    first = false;

                } else if (currentProbability + currentError >= maxOrMinProbability - error && currentProbability - currentError <= maxOrMinProbability + error) { //overlapping error intervals
              //  } else if (currentProbability == maxOrMinProbability) {
                    currentBestChildIds.push_back(currentChildId);
                    if ((currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {
                        maxOrMinProbability = currentProbability;
                        error = currentError;
                    }
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



    void computeSubTree(nondetParams* params, real_1d_array &fi, bool firstTree, double a, int &counterConstraints, int i, double &prob, double &error){


        vector<ParametricLocationTree::Node> nodes;
        if (firstTree)
            nodes = (*params).candidatesFirst;
        else
            nodes = (*params).candidatesSecond;


        //for all candidates in sub-tree
        for (ParametricLocationTree::Node node : nodes) {

            ParametricLocation location = node.getParametricLocation();
            vector<vector<pair<int, pair<vector<double>, vector<double>>>>> originalIntegrationIntervals = location.getIntegrationIntervals();
            std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> newIntegrationIntervals;

//            std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> intervalContainingA;
//
//            double minLower = originalIntegrationIntervals[0][i].second.first[0];
//            double maxUpper = originalIntegrationIntervals[0][i].second.second[0];
//
//            //for multiple interval sets, find maximal upper bound and minimal lower bound
//
//            for (int j = 1; j < originalIntegrationIntervals.size();j++){
//                if (originalIntegrationIntervals[j][i].second.first[0] <= a && originalIntegrationIntervals[j][i].second.second[0] >= a) {
//                    intervalContainingA = originalIntegrationIntervals[j];
//                    found = true;
//                    break;
//                }
//                if (originalIntegrationIntervals[j][i].second.first[0] < minLower)
//                    minLower = originalIntegrationIntervals[j][i].second.first[0];
//
//                if (originalIntegrationIntervals[j][i].second.second[0])
//            }
//
//            if (found){//
//                fi[counterConstraints] = intervalContainingA[i].second.first[0] - a; //inequality <=0
//                counterConstraints++;
//                fi[counterConstraints] = a - intervalContainingA[i].second.second[0]; //inequality <=0
//                counterConstraints++;//
//            }



            int dim = location.getDimension();
            std::vector<double> LinEqLeft(dim, 0.0);
            std::vector<double> LinEqRight(dim, 0.0);

            if (firstTree) {
                LinEqLeft[i + 1] = 1.0; // s  <=  a
                LinEqRight[0] = a;
            } else {
                LinEqLeft[0] = a;
                LinEqRight[i + 1] = 1.0; // a  <=  s
            }
            LinearEquation equation(LinEqLeft, LinEqRight);

            for (DomainWithIndex domainWithIndex : originalIntegrationIntervals) {
                Domain domain = LinearDomain::createDomain(domainWithIndex);
                LinearBoundsTree tree(domain, equation);
                std::vector<LinearDomain> domains = tree.getUniqueDomains();
                for (LinearDomain resultDomain : domains) {
                    newIntegrationIntervals.push_back(resultDomain.toDomainWithIndex(domainWithIndex));
                }
            }


            location.overwriteIntegrationIntervals(newIntegrationIntervals);


            auto calculator = new ProbabilityCalculator();
            if ((*params).algorithm == 0)
                //Gauss Legendre
                prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(location, (*params).distributions, (*params).maxTime, (*params).evaluations);
            else
                //Monte Carlo
                prob += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(location, (*params).distributions, (*params).maxTime, (*params).algorithm, (*params).functioncalls, error);

       }
    }



    void optimizationFunction(const real_1d_array &x, real_1d_array &fi, void* ptr){

        double prob = 0.0;
        double error = 0.0;
        double currentError = 0.0;
        int counterConstraints = 1;

        cout << x[0] << endl;
        nondetParams* params = (nondetParams *)ptr;

        //for all random variables
        //TODO only for fired RV with corresponding continuous place -> how to find out? for now only first variable
        int i = 1;

        computeSubTree(params, fi,  true, x[0], counterConstraints, i, prob, currentError);
        error += currentError;
        currentError = 0.0;
        computeSubTree(params, fi, false, x[0], counterConstraints, i, prob, currentError);


        fi[0] = (-1.0) * prob;

       // if (fi[1] <= 0 && fi[2] <= 0  && fi[3] <= 0 && fi[4] <= 0)
       //     cout << " TOTAL: " << a  << " --- Prob: " <<  prob << endl;

        (*params).prob = prob;
        (*params).result = error;
        (*params).result = x[0];
    }



    double NondeterminismSolver::recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error){


        double maxOrMinProbability = 0.0;
        double currentError;

        auto calculator = new ProbabilityCalculator();

        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
        vector<ParametricLocationTree::Node> conflictSet = determineConflictSet(children);


        if (conflictSet.size() >= 2) {

            real_1d_array x0 = "[0]";
            real_1d_array s = "[1]";
            double epsx = 0.01;//0.000001;
            ae_int_t maxits = 10;//0; for unlimited
            minnlcstate state; //state object
            minnlcreport rep;
            real_1d_array x1;
            double diffstep = 1.0;
            int first;
            int second;

            vector<vector<ParametricLocationTree::Node>> currentCandidatesVector;
            vector<ParametricLocationTree::Node> currentCandidates;
            //currentCandidates.clear();
            recursivelyGetCandidates(currentCandidates, conflictSet[0], candidates);
            currentCandidatesVector.push_back(currentCandidates);

            //pairwise consideration
            for (int k = 0; k < conflictSet.size(); k++) {

                for (int j = k + 1; j < conflictSet.size(); j++) {

                    currentCandidates.clear();
                    recursivelyGetCandidates(currentCandidates, conflictSet[j], candidates);
                    currentCandidatesVector.push_back(currentCandidates);

                    vector<nondetParams> params;

                    //two ways for ordering
                    for (int i = 0; i <= 1; i++) {

                        if (i == 0) {
                            first = k;
                            second = j;
                        } else {
                            first = j;
                            second = k;
                        }

                        nondetParams currentParams;
                        currentParams.algorithm = algorithm;
                        currentParams.evaluations = min(64, evaluations);
                        currentParams.functioncalls = min(1000, functioncalls);
                        currentParams.distributions = (*this->plt).getDistributions();
                        currentParams.maxTime = (*this->plt).getMaxTime();

                        currentParams.candidatesSecond = currentCandidatesVector[second];
                        currentParams.candidatesFirst = currentCandidatesVector[first];

                        minnlccreatef(1, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
                        minnlcsetalgoslp(state); //activate
                        minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
                        minnlcsetscale(state, s); //set scale (here always 1)
                        minnlcsetstpmax(state, 0.0); //settings
                        minnlcsetnlc(state, 0, 4);
                        minnlcoptimize(state, optimizationFunction, NULL, &currentParams); //start optimizer
                        minnlcresults(state, x1, rep); //get results

                        if (rep.terminationtype == 2)
                            cout << "Pre-computation " << i + 1 << " succeeded: x = " << currentParams.result << " with P_max = " << currentParams.prob << " +- " << currentParams.error << endl;
                        else
                            cout << "Pre-computation " << i + 1 << " failed" << endl;

                        params.push_back(currentParams);

                    }

                    int i;
                    if (params[0].prob >= params[1].prob) {
                        i = 0;
                        first = k;
                        second = j;
                    } else {
                        i = 1;
                        first = j;
                        second = k;
                    }

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
                        cout << "Main computation succeeded: x = " << params[i].result << " with P_max = " << params[i].prob << " +- " << params[i].error << endl;
                    else
                        cout << "Main computation failed" << endl;


                    if (conflictSet.size() == 2){

                        maxOrMinProbability = params[i].prob;
                        error = params[i].error;

                    } else {

                        //TODO test this part with appropriate example

                        double a = params[i].result;

                        //adjust upper bound for first
                        for (int l = 0; j < currentCandidatesVector[first].size(); j++){

                            ParametricLocation location = currentCandidatesVector[first][l].getParametricLocation();
                            vector<vector<pair<int, pair<vector<double>, vector<double>>>>> originalIntegrationIntervals = location.getIntegrationIntervals();

                            vector<vector<pair<int, pair<vector<double>, vector<double>>>>> newIntegrationIntervals;

                            int dim = location.getDimension();
                            std::vector<double> LinEqLeft(dim, 0.0);
                            std::vector<double> LinEqRight(dim, 0.0);

                            LinEqLeft[i + 1] = 1.0; // s  <=  a
                            LinEqRight[0] = a;

                            LinearEquation equation(LinEqLeft, LinEqRight);

                            for (DomainWithIndex domainWithIndex : originalIntegrationIntervals) {
                                Domain domain = LinearDomain::createDomain(domainWithIndex);
                                LinearBoundsTree tree(domain, equation);
                                std::vector<LinearDomain> domains = tree.getUniqueDomains();
                                for (LinearDomain resultDomain : domains) {
                                    newIntegrationIntervals.push_back(resultDomain.toDomainWithIndex(domainWithIndex));
                                }
                            }
                            currentCandidatesVector[first][l].getParametricLocation().overwriteIntegrationIntervals(newIntegrationIntervals);

                        }



                        //adjust lower bound for second
                        for (int l = 0; l < currentCandidatesVector[second].size(); l++){

                           ParametricLocation location = currentCandidatesVector[second][l].getParametricLocation();
                           vector<vector<pair<int, pair<vector<double>, vector<double>>>>> originalIntegrationIntervals = location.getIntegrationIntervals();

                           vector<vector<pair<int, pair<vector<double>, vector<double>>>>> newIntegrationIntervals;

                           int dim = location.getDimension();
                           std::vector<double> LinEqLeft(dim, 0.0);
                           std::vector<double> LinEqRight(dim, 0.0);

                           LinEqLeft[0] = a;
                           LinEqRight[i + 1] = 1.0; // a  <=  s

                           LinearEquation equation(LinEqLeft, LinEqRight);

                           for (DomainWithIndex domainWithIndex : originalIntegrationIntervals) {
                               Domain domain = LinearDomain::createDomain(domainWithIndex);
                               LinearBoundsTree tree(domain, equation);
                               std::vector<LinearDomain> domains = tree.getUniqueDomains();
                               for (LinearDomain resultDomain : domains) {
                                   newIntegrationIntervals.push_back(resultDomain.toDomainWithIndex(domainWithIndex));
                               }
                           }
                           currentCandidatesVector[second][l].getParametricLocation().overwriteIntegrationIntervals(newIntegrationIntervals);


                        }
                    }

                }
            }

            if (conflictSet.size() > 2){

                maxOrMinProbability = 0.0;

                for (vector<ParametricLocationTree::Node> nodes : currentCandidatesVector){
                    for (ParametricLocationTree::Node node : nodes){

                        if (algorithm == 0)
                            //Gauss Legendre
                            maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(node.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
                        else
                            //Monte Carlo
                            maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(node.getParametricLocation(), (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, functioncalls, currentError);

                        error += currentError;

                    }
                }
            }
        }


        //add probability for sub-trees which are not in the conflict set
        for (ParametricLocationTree::Node child: children){
            maxOrMinProbability += recursivelySolveNondeterminismProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError);
            error += currentError;
        }


    //add probability for currentNode if it is a candidate
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
