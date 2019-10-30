#include "NondeterminismSolver.h"

#include "ProbabilityCalculator.h"
#include "helper/LinearBoundsTree.h"
#include "helper/LinearDomain.h"
#include "helper/LinearEquation.h"
#include "optimization/optimization.h"

using namespace std;
using namespace alglib;

namespace hpnmg {


    NondeterminismSolver::NondeterminismSolver() {}


    //TODO diese Funktion muss später durch Janniks / Henners Model Checking sinnvoll ersetzt werden, vermutlich werden dann noch Intervalle eingeschränkt
    bool NondeterminismSolver::fulfillsProperty(ParametricLocationTree::Node node, int version){

    vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
    vector<vector<double>> continuousMarking = node.getParametricLocation().getContinuousMarking();

//    if (discreteMarking[5] > 0) //fig5prophetic + fig1prophetic
//        return true;
//
//    if (discreteMarking[4] > 0) //fig1nonprophetic + fig5nonprophetic
//        return true;


    if (version == 1 || version == 2) {

        vector<double> level = continuousMarking[0]; //charging_ver1 + charging_ver2
        bool empty = true;
        for (int p = 0; p < level.size(); p++)
            if (level[p] > 0)
                empty = false;
        if ((discreteMarking[2] > 0 && !empty && discreteMarking[1] == 0) || (discreteMarking[3] > 0 && empty && discreteMarking[1] > 0)) //right decision
            return true;

    } else if (version == 3 || version == 4) {

        vector<double> levelCar = continuousMarking[1]; //charging_ver3 + charging_ver4
        bool emptyCar = true;
        for (int p = 0; p < levelCar.size(); p++)
            if (levelCar[p] > 0)
                emptyCar = false;
        vector<double> levelDistance = continuousMarking[0];
        bool emptyDistance = true;
        for (int q = 0; q < levelDistance.size(); q++)
            if (levelDistance[q] > 0)
                emptyDistance = false;

        if ((discreteMarking[6] > 0 && !emptyCar && emptyDistance) || (discreteMarking[7] > 0 && emptyCar && !emptyDistance)) //right decision
            return true;

    }


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



    bool NondeterminismSolver::isCandidateForProperty(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates, int version){

        for (ParametricLocationTree::Node currentCandidate : candidates){

            if (currentCandidate.getNodeID() == node.getNodeID() && fulfillsProperty(currentCandidate, version)){
                node = currentCandidate;
                return true;
            }
        }

        return false;
    }


    void NondeterminismSolver::recursivelyGetCandidateLocations(vector<ParametricLocation> &list, ParametricLocationTree::Node node, std::vector<ParametricLocationTree::Node> candidates, int version){

        if (isCandidateForProperty(node, candidates, version))
            list.push_back(node.getParametricLocation());

        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(node);

        for (ParametricLocationTree::Node child : children)
            recursivelyGetCandidateLocations(list, child, candidates, version);
    }



    double NondeterminismSolver::recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error, int version){


        double maxOrMinProbability = 0.0;
        double currentProbability;
        double currentError;

        int currentChildId;
        std::string currentComponent;


        auto calculator = new ProbabilityCalculator();


        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
        vector<ParametricLocationTree::Node> conflictSet = NondeterminismSolver::determineConflictSet(children);

        if (conflictSet.size() > 0) {

            bool first = true;
            vector<pair<int, string>> currentBestChildLocations;


            for (ParametricLocationTree::Node child: conflictSet) {

                currentChildId = child.getNodeID();
                currentComponent = child.getParametricLocation().getSourceEvent().getMemberID();
                currentError = 0.0;
                currentProbability = recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError, version);

                if (first || (currentProbability - currentError > maxOrMinProbability + error && minimum == false) || (currentProbability + currentError < maxOrMinProbability - error && minimum == true)) {
                //error intervals do not overlap
                    currentBestChildLocations.clear();
                    currentBestChildLocations.push_back(make_pair(currentChildId, currentComponent));
                    maxOrMinProbability = currentProbability;
                    error = currentError;
                    first = false;

                } else if (currentProbability + currentError >= maxOrMinProbability - error && currentProbability - currentError <= maxOrMinProbability + error) {
                //overlapping error intervals
                    currentBestChildLocations.push_back(make_pair(currentChildId, currentComponent));
                    if ((currentProbability > maxOrMinProbability && minimum == false) || (currentProbability < maxOrMinProbability && minimum == true)) {
                        maxOrMinProbability = currentProbability;
                        error = currentError;
                    }
                }
            }

            (this->bestChildLocations).push_back(currentBestChildLocations);
        }



        for (ParametricLocationTree::Node child: children){
            maxOrMinProbability += recursivelySolveNondeterminismNonProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError, version);
            error += currentError;
        }


        if (isCandidateForProperty(currentNode, candidates, version)){

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



    void computeSubTree(nondetParams* params, real_1d_array &fi, bool upperBounded, double a, int rvIdnex, double &prob, double &error){


        vector<ParametricLocation> locations;
        if (upperBounded)
            locations = (*params).upperBounded;
        else
            locations = (*params).lowerBounded;


        //for all candidates in sub-tree
        for (ParametricLocation location : locations) {

            vector < vector < pair < int, pair < vector < double >, vector < double >> >> > originalIntegrationIntervals = location.getIntegrationIntervals();
            std::vector < std::vector < std::pair < int, std::pair < std::vector < double >, std::vector < double >> >> > newIntegrationIntervals;


            int dim = location.getDimension();
            std::vector<double> LinEqLeft(dim, 0.0);
            std::vector<double> LinEqRight(dim, 0.0);

            if (upperBounded) {
                LinEqLeft[rvIdnex + 1] = 1.0; // s  <=  a
                LinEqRight[0] = a;
            } else {
                LinEqLeft[0] = a;
                LinEqRight[rvIdnex + 1] = 1.0; // a  <=  s
            }
            LinearEquation equation(LinEqLeft, LinEqRight);

            for (DomainWithIndex domainWithIndex : originalIntegrationIntervals) {
                Domain domain = LinearDomain::createDomain(domainWithIndex);
                LinearBoundsTree tree(domain, equation);
                std::vector <LinearDomain> domains = tree.getUniqueDomains();
                for (LinearDomain resultDomain : domains) {
                    newIntegrationIntervals.push_back(resultDomain.toDomainWithIndex(domainWithIndex));
                }
            }

            if (newIntegrationIntervals.size() > 0) {


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
    }



    void optimizationFunction(const real_1d_array &x, real_1d_array &fi, void* ptr){

        double prob = 0.0;
        double error = 0.0;
        double currentError = 0.0;


        nondetParams* params = (nondetParams *)ptr;


        computeSubTree(params, fi,  true, x[0], (*params).rvIndex, prob, currentError);
        error += currentError;
        currentError = 0.0;
        computeSubTree(params, fi, false, x[0], (*params).rvIndex, prob, currentError);

        double m = (-1.0);
        if ((*params).minimum)
            m = 1.0;

        fi[0] = m * prob;
        fi[1] = 0 - x[0];
        fi[2] = x[0] - (*params).maxTime;

        if (x[0] >= 0.0 && x[0] <= (*params).maxTime && (((*params).minimum && prob < (*params).prob) || (!((*params).minimum) && prob > (*params).prob))){
            (*params).prob = prob;
            (*params).result = error;
            (*params).result = x[0];
        }

        //cout << x[0] << " --- " << prob <<endl;
    }



    double NondeterminismSolver::recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error, int version){


        double maxOrMinProbability = 0.0;
        error = 0.0;
        double currentError;

        auto calculator = new ProbabilityCalculator();

        vector<ParametricLocationTree::Node> children = (*this->plt).getChildNodes(currentNode);
        vector<ParametricLocationTree::Node> conflictSet = determineConflictSet(children);


        if (conflictSet.size() >= 2) {

            real_1d_array x0 = "[8]";
            real_1d_array s = "[12]";
            double epsx = 0.01;//0.000001;
            ae_int_t maxits = 0;//0; for unlimited
            minnlcstate state; //state object
            minnlcreport rep;
            real_1d_array x1;
            double diffstep = 0.1;

            int upperBounded;
            int lowerBounded;

            vector<vector<ParametricLocation>> currentLocationsVector;
            vector<ParametricLocation> currentLocations;
            recursivelyGetCandidateLocations(currentLocations, conflictSet[0], candidates, version);
            currentLocationsVector.push_back(currentLocations);

            //pairwise consideration
            for (int k = 0; k < conflictSet.size(); k++) {

                for (int j = k + 1; j < conflictSet.size(); j++) {

                    currentLocations.clear();
                    recursivelyGetCandidateLocations(currentLocations, conflictSet[j], candidates, version);
                    currentLocationsVector.push_back(currentLocations);

                    vector<nondetParams> params;

                    //two ways for ordering
//START AUSKOMMENTIEREN
                    for (int i = 0; i <= 1; i++) {

                        if (i == 0) {
                            upperBounded = k;
                            lowerBounded = j;
                        } else {
                            upperBounded = j;
                            lowerBounded = k;
                        }

                        nondetParams currentParams;
                        currentParams.algorithm = algorithm;
                        currentParams.evaluations = min(64, evaluations);
                        currentParams.functioncalls = min(1000, functioncalls);
                        currentParams.distributions = (*this->plt).getDistributions();
                        currentParams.maxTime = (*this->plt).getMaxTime();
                        currentParams.minimum = minimum;
                        //only for first fired general transition
                        currentParams.rvIndex = 0.0; //currentNode.getParametricLocation().getGeneralTransitionsFired()[0] + 1;

                        if (currentNode.getParametricLocation().getGeneralTransitionsFired().size() <= 0)
                            cout << "Error: no GT has fired yet when non-deterministic choice is taken." << endl;

                        currentParams.lowerBounded = currentLocationsVector[lowerBounded];
                        currentParams.upperBounded = currentLocationsVector[upperBounded];

                        currentParams.prob = -1.0;
                        if (minimum)
                            currentParams.prob = 2.0;
                        currentParams.result = -1.0;

                        minnlccreatef(1, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
                        minnlcsetalgoslp(state); //activate
                        minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
                        minnlcsetscale(state, s); //set scale (here always 1)
                        minnlcsetstpmax(state, 0.0); //settings
                        minnlcsetnlc(state, 0, 2);
                        minnlcoptimize(state, optimizationFunction, NULL, &currentParams); //start optimizer
                        minnlcresults(state, x1, rep); //get results

                        if (rep.terminationtype == 2) {
                            cout << "Pre-computation " << i + 1 << " succeeded: x = " << currentParams.result << " with P_max = " << currentParams.prob << " +- " << currentParams.error << endl;
                            cout << "Upper bounded: " << conflictSet[upperBounded].getNodeID() << ", lower bounded: " << conflictSet[lowerBounded].getNodeID() << endl;
                        } else
                            cout << "Pre-computation " << i + 1 << " failed" << endl;

                        params.push_back(currentParams);

                    }
//ENDE AUSKOMMENTIEREN

                    int i;
                    if (params[0].prob >= params[1].prob) {
                        i = 0;
                        upperBounded = k;
                        lowerBounded = j;
                    } else {
                        i = 1;
                        upperBounded = j;
                        lowerBounded = k;
                     }


//START AUSKOMMENTIEREN
                    params[i].functioncalls = functioncalls;
                    params[i].evaluations = evaluations;
                    params[i].prob = -1.0;
                    if (minimum)
                        params[i].prob = 2.0;
                    params[i].result = -1.0;
                    minnlccreatef(1, x0, diffstep, state); //create, use f at the end for automatic gradient calculation
                    minnlcsetalgoslp(state); //activate
                    minnlcsetcond(state, epsx, maxits); //select stopping criteria for inner iterations
                    minnlcsetscale(state, s); //set scale (here always 1)
                    minnlcsetstpmax(state, 0.0); //settings
                    minnlcsetnlc(state, 0, 2);
                    minnlcoptimize(state, optimizationFunction, NULL, &params[i]); //start optimizer
                    minnlcresults(state, x1, rep); //get results

                    if (rep.terminationtype == 2) {
                        cout << "Main computation succeeded: x = " << params[i].result << " with P_max = " << params[i].prob << " +- " << params[i].error << endl;
                        cout << "Upper bounded: " << conflictSet[upperBounded].getNodeID() << ", lower bounded: " << conflictSet[lowerBounded].getNodeID() << endl;
                    } else
                        cout << "Main computation failed" << endl;



//ENDE AUSKOMMENTIEREN

                    double a = params[i].result;

                    //adjust upper bound for first
                    for (int l = 0; l < currentLocationsVector[upperBounded].size(); l++){

                        ParametricLocation location = currentLocationsVector[upperBounded][l];
                        vector<vector<pair<int, pair<vector<double>, vector<double>>>>> originalIntegrationIntervals = location.getIntegrationIntervals();

                        vector<vector<pair<int, pair<vector<double>, vector<double>>>>> newIntegrationIntervals;

                        int dim = location.getDimension();
                        std::vector<double> LinEqLeft(dim, 0.0);
                        std::vector<double> LinEqRight(dim, 0.0);

                        LinEqLeft[1] = 1.0; // s  <=  a
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
                        currentLocationsVector[upperBounded][l].overwriteIntegrationIntervals(newIntegrationIntervals);

                    }



                    //adjust lower bound for second
                    for (int l = 0; l < currentLocationsVector[lowerBounded].size(); l++){

                       ParametricLocation location = currentLocationsVector[lowerBounded][l];
                       vector<vector<pair<int, pair<vector<double>, vector<double>>>>> originalIntegrationIntervals = location.getIntegrationIntervals();

                       vector<vector<pair<int, pair<vector<double>, vector<double>>>>> newIntegrationIntervals;

                       int dim = location.getDimension();
                       std::vector<double> LinEqLeft(dim, 0.0);
                       std::vector<double> LinEqRight(dim, 0.0);

                       LinEqLeft[0] = a;
                       LinEqRight[1] = 1.0; // a  <=  s

                       LinearEquation equation(LinEqLeft, LinEqRight);

                       for (DomainWithIndex domainWithIndex : originalIntegrationIntervals) {
                           Domain domain = LinearDomain::createDomain(domainWithIndex);
                           LinearBoundsTree tree(domain, equation);
                           std::vector<LinearDomain> domains = tree.getUniqueDomains();
                           for (LinearDomain resultDomain : domains) {
                               newIntegrationIntervals.push_back(resultDomain.toDomainWithIndex(domainWithIndex));
                           }
                       }
                       currentLocationsVector[lowerBounded][l].overwriteIntegrationIntervals(newIntegrationIntervals);


                    }
                }
            }

                maxOrMinProbability = 0.0;
                error = 0.0;

                for (vector<ParametricLocation> locations : currentLocationsVector){
                    for (ParametricLocation location : locations){

                        if (algorithm == 0)
                            //Gauss Legendre
                            maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingGauss(location, (*this->plt).getDistributions(), (*this->plt).getMaxTime(), evaluations);
                        else
                            //Monte Carlo
                            maxOrMinProbability += calculator->ProbabilityCalculator::getProbabilityForLocationUsingMonteCarlo(location, (*this->plt).getDistributions(), (*this->plt).getMaxTime(), algorithm, 1000000, currentError);

                        error += currentError;

                    }
                }

            cout << "Local probability: " << maxOrMinProbability << " +- " << error << endl;

        }


        //add probability for sub-trees which are not in the conflict set
        for (ParametricLocationTree::Node child: children){
            maxOrMinProbability += recursivelySolveNondeterminismProphetic(child, candidates, algorithm, functioncalls, evaluations, minimum, currentError, version);
            error += currentError;
        }


    //add probability for currentNode if it is a candidate
        if (isCandidateForProperty(currentNode, candidates, version)){

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



    double NondeterminismSolver::solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic, double &error, int version){

        this->plt = plt;

        if (prophetic)
            return recursivelySolveNondeterminismProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum, error, version);
        else
            return recursivelySolveNondeterminismNonProphetic(currentNode, candidates, algorithm, functioncalls, evaluations, minimum, error, version);
    }



    vector<vector<pair<int, string>>> NondeterminismSolver::getBestChildLocations(){
        return this->bestChildLocations;
    }

}
