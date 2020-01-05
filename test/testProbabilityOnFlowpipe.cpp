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

#include <bits/stdc++.h>
using namespace hpnmg;
using namespace std;

std::tuple<bool, string> Check1(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 9 && point[startIndex+1] >= 4, "x >= 9 and y >= 4");
}

std::tuple<bool, string> Check2(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 9 && point[startIndex+1] <= 1, "x >= 9 and y <= 1");
}

std::tuple<bool, string> Check3(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 8, "x >= 8");
}

std::tuple<bool, string> Check4(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex+1] >= 4, "y >= 4");
}

std::tuple<bool, string> Check5(int startIndex, Point<Number> point) {
	return  std::tuple(point[startIndex] >= 2* point[startIndex+1], "x >= 2*y");
}

TEST(ProbabilityFlowpipe, probability) {

	using namespace hypro;
	typedef HPolytope<Number> Representation;

	ReadHybridPetrinet reader;
	SingularAutomatonCreator transformer;
	SingularAutomatonWriter automatonWriter;

// setup
	string filePath = "../../test/testfiles/Lisa2.xml";
	double tMax = 20 .0;

// read HPnG
	shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

// transform HPnG into SingularAutomaton
	auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));

// Get and export PLT (not necessary for reachability)
	shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
	auto writer = new PLTWriter();
	writer->writePLT(plt, tMax);

// Pass Singular Automaton to Handler
	shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
	HybridAutomatonHandler handler(automaton, tMax);
	//automatonWriter.writeAutomaton(automaton, "Lisa2");
	ProbabilityOnFlowpipe p;
// Calculate probabilty for propert y
	std::vector<double> res = p.CalculateProbabiltyForPropertyUsingFunction(automaton,plt->getDistributionsNormalized(),tMax,{Check1, Check2, Check3, Check4, Check5});

}
