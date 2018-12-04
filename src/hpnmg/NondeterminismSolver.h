#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Event.h"
#include "ProbabilityCalculator.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>


namespace hpnmg {


    class NondeterminismSolver {


    private:

        shared_ptr<ParametricLocationTree> plt;

        bool fulfillsProperty(ParametricLocationTree::Node node);

        vector<ParametricLocationTree::Node> determineConflictSet(std::vector<ParametricLocationTree::Node> &children);

        bool isCandidate(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates);

        double recursivelySolveNondeterminism(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations);


    public:

        NondeterminismSolver();

        double solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations);



    };
}
