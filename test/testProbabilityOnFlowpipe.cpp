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

using namespace hpnmg;
using namespace std;

bool Check1(int startIndex, Point<Number> point) {
	return  point.rawCoordinates()[startIndex] >= 9;
}

TEST(ProbabilityFlowpipe, probability) {

	using namespace hypro;

	ProbabilityOnReachableSets p;
	ReadHybridPetrinet reader;

	string filePath = "../../test/testfiles/examplesHybridAutomata/exampleHybrid2.xml";

	shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

	double tMax = 20.0;
	p.calculateProbabilityOnReachableSet(hybridPetriNet, {Check1}, {"x >= 9"},tMax);
}
