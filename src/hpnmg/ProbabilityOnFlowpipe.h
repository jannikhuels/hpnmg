#pragma once

#include <datastructures/SingularAutomaton.h>

#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"

using namespace std;
using namespace hypro;

namespace hpnmg {

	class ProbabilityOnFlowpipe {

	public:

		bool CheckIfRelevant(Point<double> toCheck, Point<double> ref, std::vector<int> transitions);

		std::vector<double> CalculateProbabiltyForPropertyUsingFunction(shared_ptr<SingularAutomaton> automaton,
																				vector<pair<string, map<string,
																				float>>> distributions, double tMax,
																				std::vector<std::function<std::tuple<bool,string>(int,Point<Number>)>>properties);

		void PrintResults(std::vector<string> properties, std::vector<double> res);

		double CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton,
											  vector<pair<string, map<string, float>>> distributions, double tMax,
											  std::vector<int> continiousPlace, std::vector<string> op,
											  std::vector<double> value);

		bool CompareValuesWithOp(string op, double val1, double val2);

	};

}
