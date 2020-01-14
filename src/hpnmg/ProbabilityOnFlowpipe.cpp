#include "HybridAutomatonHandler.h"
#include <ProbabilityCalculator.h>
#include "ProbabilityOnFlowpipe.h"
#include "modelchecker/RegionModelChecker.h"
#include <tuple>

#include <PLTWriter.h>
#include <SingularAutomatonCreator.h>

#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;
using namespace hypro;

namespace hpnmg {

	std::pair<double, double> ProbabilityOnFlowpipe::CalculateProbability(std::vector<std::vector<Point<double >>> polytopes, vector<pair<string, map<string, float>>> distributions) {
		ProbabilityCalculator probability;
		double totalError = 0.0;
		std::vector<hypro::HPolytope<double>> poly = CreateHPolytopes(polytopes);
		auto res = probability.getProbabilityForUnionOfPolytopesUsingMonteCarlo(poly, distributions, 1, 5000, totalError);
		return std::pair(res,totalError);
	}

	std::vector<std::vector<Point<double>>> ProbabilityOnFlowpipe::ComputePolytopesWithProperty(std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes,std::function<std::tuple<bool,string>(int, Point<Number>)> property ,std::vector<int> transitions) {
		std::set<Point<double>> newPoly;
		std::vector<std::vector<Point<double>>> allPolytopes;
		for (auto & indexPair: flowpipes) {
			std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
			// Iterate over flowpipes
			for (auto & set: flowpipe) {
				std::vector<Point<Number>> points = set.vertices();
				// Filter points of boxes wether property is satisfied
				if (!points.empty() && points.size() >= 0) {
					for (auto & point: points) {
						auto check = property(transitions.size(), point);
						if (std::get<0>(check)) {
							std::vector<double> coordinates;
							// Convert points to type double
							for (int i: transitions) {
								auto coordinate = carl::convert<Number, double>(point.rawCoordinates()[i]);
								coordinates.push_back(coordinate);
							}
							Point<double> newPoint = Point<double>(coordinates);
							newPoly.insert(newPoint);
						}
					}
				}
			}
			if (newPoly.size() > 0) {
				allPolytopes.push_back(std::vector<Point<double>>(newPoly.begin(), newPoly.end()));
				newPoly.clear();
			}
		}
		return allPolytopes;
	}

	std::vector<std::pair<double,double>>  ProbabilityOnFlowpipe::CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<int> continuousPlace, std::vector<string> op, std::vector<double> value, double dfactor) {

		std::vector<int> index = std::vector<int>(continuousPlace.size());
		for (int i = 0; i < continuousPlace.size(); i++) {
			index[i] = automaton -> getInitialGeneral().size() + continuousPlace[i];
		}

    	std::function<std::tuple<bool, string> (int, Point<Number>)> lambda = [=](int transitions,Point<Number> point){

			bool satisfied = true;
			string prop = "";
			for (int i = 0; i < value.size(); i++) {
				prop += to_string(continuousPlace[i]);
				prop+= op[i];
				prop+=to_string(value[i]);
				if(i+1< value.size()) prop += "&";
				auto coordinate = index[i];
				satisfied &= CompareValuesWithOp(op[i], carl::convert<Number, double>(point.rawCoordinates()[coordinate]), value[i]);}
			if(transitions == -1) return std::tuple(false,prop);
			return std::tuple(satisfied, prop); };
		return CalculateProbabiltyForProperty(automaton, distributions, tMax,{lambda}, dfactor);
    }

	std::vector<std::pair<double,double>> ProbabilityOnFlowpipe::CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<std::function<std::tuple<bool,string>(int, Point<Number>)>> properties, double dfactor) {

		HybridAutomatonHandler handler(automaton, tMax);
		// Compute flowpipes
		std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes = handler.computeFlowpipes(tMax, dfactor, 5);
		//handler.plotTex("HA2_Uniform", flowpipes);
		// indizes of all generall transistions
		std::vector<int> transitions = std::vector<int>(automaton -> getInitialGeneral().size());
		std::iota(transitions.begin(), transitions.end(), 0);
		string printout;
		std::vector<std::pair<double,double>> results;
		std::vector<string> propoutput;
		std::tuple<bool,string> check;
		//Compute convex
		for(auto prop : properties) {
			std::vector<std::vector<Point<double>>> allPolytopes = ComputePolytopesWithProperty(flowpipes,prop,transitions);
			PrintPolytopes(allPolytopes);
			auto res = CalculateProbability(allPolytopes,distributions);
			Point<Number> p = Point<Number>({0.1});
			propoutput.push_back(std::get<1>(prop(-1,p)));
			results.push_back(res);
		}
		PrintResults(propoutput, results);
		return results;
	}


	double ProbabilityOnFlowpipe::CalculateDiscretizedAverage(HybridPetrinet hpng,double maxtime, double dfactor, const Formula &formula) {
    	RegionModelChecker modelchecker(hpng,maxtime);
    	int timepoints = maxtime/dfactor;

    	double result = 0.0;
    	double error = 0.0;
    	double time = 0.0;
    	for(int i = 0; i <timepoints; i+= 1) {
    		std::pair<double,double> res = modelchecker.satisfies(formula, time);
    	    result += (res.first * 1/timepoints);
    	    error += res.second;
    		time += dfactor;
    	}
    	result /= timepoints;
    	return result;
    }

	std::vector<std::pair<double,double>>  ProbabilityOnFlowpipe::CalculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<std::function<std::tuple<bool,string>(int, Point<Number>)>> properties, double dfactor,double tMax) {
		SingularAutomatonCreator transformer;
		auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));
		shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
		auto writer = new PLTWriter();
		writer->writePLT(plt, tMax);
		shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
		return CalculateProbabiltyForProperty(automaton,plt->getDistributionsNormalized(),tMax,properties,dfactor);
	}

	std::vector<std::pair<double,double>>ProbabilityOnFlowpipe::CalculateProbabilityOnReachableSet(shared_ptr<HybridPetrinet>  hybridPetriNet, std::vector<int> continuousPlace, std::vector<string> op, std::vector<double> value, double dfactor,double tMax) {
		SingularAutomatonCreator transformer;
		auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));
		shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
		auto writer = new PLTWriter();
		writer->writePLT(plt, tMax);
		shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
		return CalculateProbabiltyForProperty(automaton,plt->getDistributionsNormalized(),tMax,continuousPlace,op,value,dfactor);
	}

	void ProbabilityOnFlowpipe::PrintResults(std::vector<string> properties, std::vector<std::pair<double,double>> res) {

    	for(int i = 0; i < properties.size(); i++) {
    		cout << "[Property] " << properties[i] << " [Probability] " << res[i].first << " [Error] " <<res[i].second  << endl;
    	}
    }

    void ProbabilityOnFlowpipe::PrintPolytopes(std::vector<std::vector<Point<double>>> polytopes) {
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

	std::vector<hypro::HPolytope<double>>  ProbabilityOnFlowpipe::CreateHPolytopes(std::vector<std::vector<Point<double>>> polytopes){
		std::vector<hypro::HPolytope<double>> hpolytopes;
		for (int i = 0; i < polytopes.size(); i++) {
			std::vector<Point<double >> currentPoints = polytopes[i];
			hypro::HPolytope<double> polytope = hypro::HPolytope<double> (currentPoints);
			hpolytopes.push_back(polytope);
		}
		return hpolytopes;
    }

    bool ProbabilityOnFlowpipe::CompareValuesWithOp(string op, double val1, double val2) {
        if (op == "<") return val1 < val2;
        if (op == "<=") return val1 <= val2;
        if (op == "==") return val1 == val2;
        if (op == ">") return val1 > val2;
        if (op == ">=") return val1 >= val2;
    }



}