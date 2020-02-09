#include <HybridAutomatonHandler.h>
#include <ProbabilityCalculator.h>
#include <ProbabilityOnReachableSets.h>
#include <modelchecker/RegionModelChecker.h>
#include <tuple>
#include <PLTWriter.h>
#include <SingularAutomatonCreator.h>
#include <SingularAutomatonWriter.h>
#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;
using namespace hypro;

namespace hpnmg {

	//defines which Monte Carlo algorithm will be used
	char algorithm = 1;
	//amount of function calls for the Monte Carlo algorithm
	int functioncalls = 5000;
	//discretization factor for flowpipe construction
	double dfactor = 0.01;
	//jump depth needed for flowpipe construction
	int jumpDepth = 5;

	/**
	 * Computes the probability that the given properties are fulfilled for a random state and timepoint
	 * In the future: Compute time-bounded reachability! (-> probability per flowpipe segment not yet implemented)
	 * @param hybridPetriNet the net to calculate the probability for
	 * @param properties the properties to check
	 * @param printProperties the strings corrensponding to the properties (only for console output)
	 * @param tMax time horizon for the PLT and flowpipe construction
	 * @return probabilities for the given properties
	 */
	std::vector<std::pair<double,double>>  ProbabilityOnReachableSets::calculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<std::function<bool(int, Point<Number>)>> properties, std::vector<string> printProperties, double tMax) {
		SingularAutomatonCreator transformer;
		SingularAutomatonWriter w;

		//calculate parametric location tree and singular automaton
		auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));
		shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
		auto writer = new PLTWriter();
		writer->writePLT(plt, tMax);
		auto distributions = plt->getDistributionsNormalized();
		shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
		//if wanted: w.writeAutomaton(automaton,"singularAutomaton");

		HybridAutomatonHandler handler(automaton, tMax);

		// Compute flowpipes
		std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes = handler.computeFlowpipes(tMax, dfactor, jumpDepth);
		//if wanted:
		//handler.plotTex("flowpipe_results", flowpipes);

		std::vector<std::pair<double,double>> results;


		// for each property: compute polytopes that fulfil the property and calculate the corresponding probability
		for(int i = 0; i < properties.size(); i++) {
			auto allPolytopes = computePolytopesThatFulfilProperty(flowpipes,properties[i],automaton -> getInitialGeneral().size());
			
			auto res = calculateProbabilityForPolytopes(allPolytopes,distributions);
			results.push_back(res);
		}

		printResults(printProperties, results);
		return results;
	}


	/**
	 * Computes the probability that a given property is fulfilled for a random state and timepoint
	 * In the future: Compute time-bounded reachability! (-> probability per flowpipe segment not yet implemented)
	 * @param hybridPetriNet the net to calculate the probability for
	 * @param propPlaces the indizes of the places to check
	 * @param propOps the operations for each place to each
	 * @param propValues the values to check the places against
	 * @param conjunction if the single operations should be conjoint or not
	 * @param printProperty the string corrensponding to the property (only for console output)
	 * @param tMax time horizon for the PLT and flowpipe construction
	 * @return probability for the given property
	 * @param tMax
	 */
	std::vector<std::pair<double,double>> ProbabilityOnReachableSets::calculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<int> propPlaces, std::vector<string> propOps, std::vector<double> propValues, bool conjunction, string printProperty, double tMax) {
		SingularAutomatonCreator transformer;
		SingularAutomatonWriter w;

		//calculate parametric location tree and singular automaton
		auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));
		shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
		auto writer = new PLTWriter();
		writer->writePLT(plt, tMax);
		auto distributions = plt->getDistributionsNormalized();
		shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
		//if wanted: w.writeAutomaton(automaton,"singularAutomaton");

		//transform given property into a function
		auto property = transformPropertyIntoFunction(automaton->getInitialGeneral().size(),automaton->getInitialContinuous().size(), propPlaces, propOps,propValues,conjunction);

		HybridAutomatonHandler handler(automaton, tMax);

		// Compute flowpipes
		std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes = handler.computeFlowpipes(tMax, dfactor, jumpDepth);
		//if wanted: handler.plotTex("flowpipe_results", flowpipes);

		//compute polytopes that fulfil the property
		auto allPolytopes = computePolytopesThatFulfilProperty(flowpipes,property,automaton -> getInitialGeneral().size());

		//compute the probability over the polytop
		auto result = calculateProbabilityForPolytopes(allPolytopes,distributions);

		printResults({printProperty}, {result});
		return {result};
	}



	/**
	 * Computes a polytope that fulfils a given property for each flowpipe segment.
	 * The points that make up the polytope are reduced to the general transition values.
	 * @param flowpipes results of the reachability analysis
	 * @param property property to check each point for
	 * @param transitions number of general transition values
	 * @return HPolytope for each flowpipe segment that fulfils the given property
	 */
	std::vector<hypro::HPolytope<double>> ProbabilityOnReachableSets::computePolytopesThatFulfilProperty(std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes, std::function<bool(int, Point<Number>)> property , int transitions) {
		std::set<Point<double>> newPoly;
		std::vector<std::vector<Point<double>>> allPolytopes;

		//iterate over the flowpipe segments
		for (auto & indexPair: flowpipes) {
			std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
			//iterate over the single points of the flowpipe
			for (auto & set: flowpipe) {
				std::vector<Point<Number>> points = set.vertices();
				if (!points.empty() && points.size() >= 0) {
					for (auto & point: points) {
						//filter points for given property
						if (property(transitions, point)) {
							//if point fulfils property reduce to general transitions only and add to set representing the new polytope
							std::vector<double> coordinates;
							for (int i = 0; i < transitions; i++){
								auto coordinate = carl::convert<Number, double>(point.rawCoordinates()[i]);
								coordinates.push_back(coordinate);
							}
							Point<double> newPoint = Point<double>(coordinates);
							newPoly.insert(newPoint);
						}
					}
				}
			}
			//if at least one point in the current flowpipe segment was added to the set transform the set into a vector
			if (newPoly.size() > 0) {
				allPolytopes.push_back(std::vector<Point<double>>(newPoly.begin(), newPoly.end()));
				newPoly.clear();
			}
		}
		//if wanted: printPointsOfPolytopes(allPolytopes);
		//transform the vectors of points representing the polytopes to HPolytopes
		std::vector<hypro::HPolytope<double>> polytopes = createHPolytopes(allPolytopes);
		return polytopes;
	}

	/**
	 * Calculates the probability for a given set of polytopes and the given distritbutions for the general transitions.
	 * @param polytopes vector of points that build up the polytope - one for each flowpipe segment
	 * @param distributions distributions for the general transtions
	 * @return probability that the general transition fire at time points included in the polytopes and an error value
	 */
	std::pair<double, double> ProbabilityOnReachableSets::calculateProbabilityForPolytopes(std::vector<hypro::HPolytope<double>> polytopes, vector<pair<string, map<string, float>>> distributions) {
		ProbabilityCalculator probability;
		double error = 0.0;
		double totalError = 0.0;
		double res = 0.0;
		for(auto p : polytopes) {
			//Missing: Multiply the probability for each flowpipe segment with the probability to reach the flowpipe segment
			res += probability.getProbabilityForUnionOfPolytopesUsingMonteCarlo({p}, distributions, algorithm, functioncalls, error);
			totalError += error;
		}

		return std::pair(res,totalError);
	}

	/**
	 * Transforms a property given in the form 0 <= 9 | 1 >= 1 (0 and 1 indizes of continuous places) into a funtion
	 * @param general amount of general transition values in the automaton
	 * @param continuous amount of continuous places in the automaton
	 * @param propPlaces the indizes of the places to check
	 * @param propOps the operations for each place to each
	 * @param propValues the values to check the places against
	 * @param conjunction if the single operations should be conjoint or not
	 * @return function that corresponds to the given property
	 */
	std::function<bool(int, Point<Number>)> ProbabilityOnReachableSets::transformPropertyIntoFunction(int general, int continuous, vector<int> propPlaces, std::vector<string> propOps, std::vector<double> propValues, bool conjunction) {
		//check if property is valid
		if(propPlaces.size() != propOps.size() || propPlaces.size() != propValues.size() || propOps.size() != propValues.size()) {
			cout << "The given property is not valid. Please make sure that each place has a corresponding value and operation!" << endl;
			return NULL;
		}

		//calculate the indizes for the continuous places that want to get checked agains the values
		std::vector<int> index = std::vector<int>(continuous);
		for (int i = 0; i < propPlaces.size(); i++) {
			index[i] = general + propPlaces[i];
		}

		//create lambda function that corresponds to the given property
		std::function<bool(int, Point<Number>)> lambda = [=](int transitions,Point<Number> point){
			bool satisfied = conjunction;
			for (int i = 0; i < propValues.size(); i++) {
				//checks wether the operation at index i is fulfilled for the corresponding place and value
				auto coordinate = index[i];
				auto satiesfiedPart =  compareValuesWithOperation(propOps[i], carl::convert<Number, double>(point.rawCoordinates()[coordinate]), propValues[i]);
				conjunction ? satisfied &= satiesfiedPart : satisfied |= satiesfiedPart;
			}
			return satisfied;
		};

		return lambda;

	}

	/**
	 * Creates an HPolytope from a given vector of points per flowpipe segment
	 * @param polytopes vector of points per flowpipe segment
	 * @return HPolytope per flowpipe segment
	 */
	std::vector<hypro::HPolytope<double>>  ProbabilityOnReachableSets::createHPolytopes(std::vector<std::vector<Point<double>>> polytopes){
		std::vector<hypro::HPolytope<double>> hpolytopes;
		for (int i = 0; i < polytopes.size(); i++) {
			std::vector<Point<double >> currentPoints = polytopes[i];
			hypro::HPolytope<double> polytope = hypro::HPolytope<double> (currentPoints);
			hpolytopes.push_back(polytope);
		}
		return hpolytopes;
	}

	/**
	 * Translates the given string into an operation.
	 * @param op the operation used to compare the values
	 * @param place the value of the continuous place
	 * @param value the value to check against
	 * @return result of the operation, if operation is invalid false is returned in all cases
	 */
	bool ProbabilityOnReachableSets::compareValuesWithOperation(string op, double place, double value) {
		if (op == "<") return place < value;
		if (op == "<=") return place <= value;
		if (op == "==") return place == value;
		if (op == ">") return place > value;
		if (op == ">=") return place >= value;
		else return false;
	}

	/** PRINT FUNCTIONS **/

	/**
	 * Prints the calculated properties for each property
	 * @param properties names of the properties
	 * @param res probabilities and error values for each property
	 */
	void ProbabilityOnReachableSets::printResults(std::vector<string> properties, std::vector<std::pair<double,double>> res) {
		for(int i = 0; i < properties.size(); i++) {
			cout << "[Property] " << properties[i] << " [Probability] " << res[i].first << " [Error] " <<res[i].second  << endl;
		}
	}

	/**
	 * Prints all points that have been added to a polytope
	 * @param polytopes vector of points per polytope
	 */
	void ProbabilityOnReachableSets::printPointsOfPolytopes(std::vector<std::vector<Point<double>>> polytopes) {
		for (int i = 0; i < polytopes.size(); i++) {
			cout << "Polytope Nr. " << (i + 1) << endl;
			for (int j = 0; j < polytopes[i].size(); j++) {
				for (int k = 0; k < polytopes[i][j].rawCoordinates().size(); k++) {
					cout << polytopes[i][j].rawCoordinates()[k] << ", ";
				}
				cout << endl;
			}
		}
	}

}