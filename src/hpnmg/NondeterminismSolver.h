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
        vector<ParametricLocationTree::Node> candidatesFirst;
        vector<ParametricLocationTree::Node> candidatesSecond;
        vector<pair<string, map<string, float>>>  distributions;
        double maxTime;
        double prob;
        double error;
        double result;
        int functioncalls;
        char algorithm;
        int evaluations;
   	} nondetParams;


    class NondeterminismSolver {


    private:

        shared_ptr<ParametricLocationTree> plt;
        vector<vector<int>> bestChildIds;

        bool fulfillsProperty(ParametricLocationTree::Node node);

        vector<ParametricLocationTree::Node> determineConflictSet(std::vector<ParametricLocationTree::Node> &children);

        bool isCandidateForProperty(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates);

        double recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error);

        double recursivelySolveNondeterminismProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error);

        void recursivelyGetCandidates(vector<ParametricLocationTree::Node> &list, ParametricLocationTree::Node node, std::vector<ParametricLocationTree::Node> candidates);


    public:

        NondeterminismSolver();

        double solveNondeterminism(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic, double &error);

        vector<vector<int>> getBestChildIds();

    };

    void computeSubTree(nondetParams* params, alglib::real_1d_array &fi, bool firstTree, double a, int &counterConstraints, int i, double &prob, double &error);

    void optimizationFunction(const alglib::real_1d_array &x, alglib::real_1d_array &fi, void *ptr);


}
