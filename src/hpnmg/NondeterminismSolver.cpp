#include "NondeterminismSolver.h"

using namespace std;

namespace hpnmg {


    NondeterminismSolver::NondeterminismSolver() {}


    //TODO diese Funktion muss später durch Janniks Model Checking sinnvoll ersetzt werden
    bool NondeterminismSolver::fulfillsProperty(ParametricLocationTree::Node node){

    std::vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
    if (discreteMarking[4] > 0)
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



    double NondeterminismSolver::recursivelySolveNondeterminism(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum){


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
                currentProbability = recursivelySolveNondeterminism(child, candidates, algorithm, functioncalls, evaluations, minimum);

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
            maxOrMinProbability += recursivelySolveNondeterminism(child, candidates, algorithm, functioncalls, evaluations, minimum);


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



    double NondeterminismSolver::solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum){

        this->plt = plt;

        return recursivelySolveNondeterminism(currentNode, candidates, algorithm, functioncalls, evaluations, minimum);

    }


    vector<vector<int>> NondeterminismSolver::getBestChildIds(){
        return this->bestChildIds;
    }

}
