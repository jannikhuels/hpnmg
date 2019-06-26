#pragma once

#include "ParametricLocationTree.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Event.h"
#include "ProbabilityCalculator.h"
#include "helper/LinearBoundsTree.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "../src/hpnmg/optimization/stdafx.h"
#include "../src/hpnmg/optimization/optimization.h"



namespace hpnmg {

    typedef struct {
        vector<ParametricLocation> upperBounded;
        vector<ParametricLocation> lowerBounded;
        vector<pair<string, map<string, float>>>  distributions;
        double maxTime;
        double prob;
        double error;
        double result;
        int functioncalls;
        char algorithm;
        int evaluations;
        int rvIndex;
        bool minimum;
   	} nondetParams;


    class NondeterminismSolver {


    private:

        shared_ptr<ParametricLocationTree> plt;
        vector<vector<pair<int, string>>> bestChildLocations;

        bool fulfillsProperty(ParametricLocationTree::Node node, int version);

        vector<ParametricLocationTree::Node> determineConflictSet(std::vector<ParametricLocationTree::Node> &children);

        bool isCandidateForProperty(ParametricLocationTree::Node &node, std::vector<ParametricLocationTree::Node> candidates, int version);

        double recursivelySolveNondeterminismNonProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error, int version);

        double recursivelySolveNondeterminismPartiallyProphetic(ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error, int version);

        double solveNondeterminismFullyProphetic(ParametricLocationTree::Node root, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, double &error, int version);

        void recursivelyGetGoalLocations(std::vector<ParametricLocation> &goalLocations, ParametricLocationTree::Node currentNode, std::vector<ParametricLocationTree::Node> candidates, int version);


    public:

        NondeterminismSolver();

        double solveNondeterminism(shared_ptr<ParametricLocationTree> plt, std::vector<ParametricLocationTree::Node> candidates, char algorithm, int functioncalls, int evaluations, bool minimum, bool prophetic, double &error, int version, bool partially = false);

        vector<vector<pair<int, string>>> getBestChildLocations();

    };

    void computeSubTree(nondetParams* params, alglib::real_1d_array &fi, bool upperBounded, double a, int rvIdnex, double &prob, double &error);

    void optimizationFunction(const alglib::real_1d_array &x, alglib::real_1d_array &fi, void *ptr);


}
