#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Event.h"
#include "ProbabilityCalculator.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>

#include "../src/hpnmg/optimization/stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../src/hpnmg/optimization/optimization.h"


namespace hpnmg {

    typedef struct {
        vector<vector<ParametricLocationTree::Node>> orderedCandidates;
        vector<pair<string, map<string, float>>>  distributions;
        double maxTime;
   	} nondetParams;


    class NondeterminismSolver {


    private:

        shared_ptr<ParametricLocationTree> plt;
        vector<vector<int>> bestChildIds;

        bool fulfillsProperty(ParametricLocationTree::Node node);

        vector<ParametricLocationTree::Node> determineConflictSet(std::vector<ParametricLocationTree::Node> &children);

        bool isCandidate(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates);

        double recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum);

        double recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum);

        void recursivelyGetCandidates(vector<ParametricLocationTree::Node> &list, ParametricLocationTree::Node node, std::vector<ParametricLocationTree::Node> candidates);


    public:

        NondeterminismSolver();

        double solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic);

        vector<vector<int>> getBestChildIds();

    };

    void optimizationFunction(const alglib::real_1d_array &x, alglib::real_1d_array &fi, void *ptr);
}
