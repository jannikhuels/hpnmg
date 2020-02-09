#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>
#include <ProbabilityCalculator.h>
#include "ParametricLocationTree.h"
#include <SingularAutomatonCreator.h>
#include <SingularAutomatonWriter.h>
#include <HybridAutomatonHandler.h>
#include <ProbabilityOnReachableSets.h>
#include "gtest/gtest.h"
#include "PLTWriter.h"
#include <chrono>
#include <tuple>
#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "datastructures/Point.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"
#include "parser/antlr4-flowstar/ParserWrapper.h"
#include <ReadFormula.h>
#include <modelchecker/RegionModelChecker.h>
#include <bits/stdc++.h>
using namespace hpnmg;
using namespace std;

bool Check1(int startIndex, Point<Number> point) {
	return  point.rawCoordinates()[startIndex] >= 9;
}

bool Check2(int startIndex, Point<Number> point) {
	return  point.rawCoordinates()[startIndex] <= 7;
}

bool Check3(int startIndex, Point<Number> point) {
	return  point.rawCoordinates()[startIndex] <= 10 && point.rawCoordinates()[startIndex] >= 8;
}

TEST(ProbabilityOnReachableSets, functionbased) {

	using namespace hypro;

	ProbabilityOnReachableSets p;
	ReadHybridPetrinet reader;

	string filePath = "../../test/testfiles/paulineExamples/examplePauline1.xml";

	shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

	double tMax = 20.0;

	p.calculateProbabilityOnReachableSet(hybridPetriNet, {Check1, Check2, Check3}, {"x >= 9", "x <= 7", "x >=8 && x <= 10"},tMax);
}

TEST(ProbabilityOnReachableSets, atomicbased) {

	using namespace hypro;

	ProbabilityOnReachableSets p;
	ReadHybridPetrinet reader;

	string filePath = "../../test/testfiles/paulineExamples/examplePauline1.xml";

	shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

	double tMax = 10.0;
	std::vector<int> propContinuous = {0,0};
	std::vector<string> propOps = {">=", "<="};
	std::vector<double> propValues = {8.0, 10.0};
	bool conjunction = true;

	p.calculateProbabilityOnReachableSet(hybridPetriNet, propContinuous, propOps, propValues, conjunction, "x >= 8 && x <= 10",tMax);
}