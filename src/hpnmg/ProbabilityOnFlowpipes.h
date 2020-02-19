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


using namespace hypro;

namespace hpnmg {
    using namespace std;

	class ProbabilityOnFlowpipes {

	public:

		std::vector<std::pair<double,double>> computeProbabilityOnFlowpipesWithoutReset(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<std::function<bool(int, Point<Number>)>> properties, std::vector<string> printProperties, double tMax);

        std::vector<std::pair<double,double>>  computeProbabilityOnFlowpipes(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<std::function<bool(int, Point<Number>)>> properties, std::vector<string> printProperties, double tMax);

		std::vector<std::pair<double,double>> computeProbabilityOnFlowpipesWithoutReset(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<int> propPlaces, std::vector<string> propOps, std::vector<double> propValues, bool conjunction, string printProperty, double tMax);

		// Computes a polytope that fulfills a given property for each flowpipe segment.
		std::vector<hypro::HPolytope<double>> computePolytopesThatFulfillProperty(std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes,std::function<bool(int, Point<Number>)> property ,int transitions);

		// Calculates the probability for a given set of polytopes and the given distritbutions for the general transitions.
		std::pair<double, double> computeProbabilityForPolytopes(std::vector<hypro::HPolytope<double>> polytopes, vector<pair<string, map<string, float>>> distributions);

		// Transforms a property given in the form 0 <= 9 | 1 >= 1 (0 and 1 indizes of continuous places) into a function
		std::function<bool(int, Point<Number>)> transformPropertyIntoFunction(int general, int continuous, vector<int> propPlaces, std::vector<string> propOps, std::vector<double> propValues, bool conjunction);

		// Translates a given string into an operation.
		bool compareValuesWithOperation(string op, double place, double value);

		std::vector<hypro::HPolytope<double>> createHPolytopes(std::vector<std::vector<Point<double>>> polytopes);

		void printResults(std::vector<string> properties, std::vector<std::pair<double,double>> res);

		void printPointsOfPolytopes(std::vector<std::vector<Point<double>>> polytopes);

	};

}
