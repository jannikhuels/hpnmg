#pragma once

#include <datastructures/SingularAutomaton.h>

#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"
#include <modelchecker/Formula.h>

using namespace std;
using namespace hypro;

namespace hpnmg {

	class ProbabilityOnFlowpipe {

	public:

		std::pair<double, double> CalculateProbability(std::vector<std::vector<Point<double >>> polytopes, vector<pair<string, map<string, float>>> distributions);

		std::vector<std::vector<Point<double>>> ComputePolytopesWithProperty(std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes,std::function<std::tuple<bool,string>(int, Point<Number>)> property ,std::vector<int> transitions);

		std::vector<std::pair<double,double>> CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<int> continuousPlace, std::vector<string> op, std::vector<double> value, double dfactor);

		std::vector<std::pair<double,double>> CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<std::function<std::tuple<bool,string>(int, Point<Number>)>> properties, double dfactor);

		double CalculateDiscretizedAverage(HybridPetrinet hpng,double maxtime, double dfactor, const Formula &formula);

		std::vector<std::pair<double,double>> CalculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<std::function<std::tuple<bool,string>(int, Point<Number>)>> properties, double dfactor = 0.01,double tMax= 20);

		std::vector<std::pair<double,double>> CalculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<int> continuousPlace, std::vector<string> op, std::vector<double> value, double dfactor = 0.01,double tMax= 20);

		void PrintResults(std::vector<string> properties, std::vector<std::pair<double,double>> res) ;

		void PrintPolytopes(std::vector<std::vector<Point<double>>> polytopes) ;

		std::vector<hypro::HPolytope<double>>  CreateHPolytopes(std::vector<std::vector<Point<double>>> polytopes);

		bool CompareValuesWithOp(string op, double val1, double val2);


	};

}
