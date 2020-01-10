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
#include <ProbabilityOnFlowpipe.h>
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

std::tuple<bool, string> Check1(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 9, "x >= 9");
}

std::tuple<bool, string> Check2(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] <= 7 , "x <= 7");
}

std::tuple<bool, string> Check3(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 8 && point[startIndex] <= 10, "x >= 8 && x <= 10");
}

std::tuple<bool, string> Check4(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex+1] >= 4, "y >= 4");
}

std::tuple<bool, string> Check5(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 2* point[startIndex+1], "x >= 2*y");
}

TEST(ProbabilityFlowpipe, probability) {

	using namespace hypro;

	ProbabilityOnFlowpipe p;
	ReadHybridPetrinet reader;

	string filePath = "../../test/testfiles/examplePauline1.xml";

	shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

	double dfactor = 0.01;
	double tMax = 20.0;
	p.CalculateProbabilityOnReachableSet(hybridPetriNet, {Check1}, dfactor,tMax);
}
