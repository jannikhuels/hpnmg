#include "HybridAutomatonHandler.h"
#include <ProbabilityCalculator.h>
#include "ProbabilityOnFlowpipe.h"
#include <tuple>

#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;
using namespace hypro;

namespace hpnmg {

    double ProbabilityOnFlowpipe::CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<int> continiousPlace, std::vector<string> op, std::vector<double> value) {
    //get Petri net from file

        HybridAutomatonHandler handler(automaton, tMax);
        // Compute flowpipes
        std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes = handler.computeFlowpipes(tMax, 0.01, 5);
        // indizes of all generall transistions
        std::vector<int> transitions = std::vector<int>(automaton -> getInitialGeneral().size());
        std::iota(transitions.begin(), transitions.end(), 0);
        // Defines which coordinate is the one to check property for
        std::vector<int> index = std::vector<int>(continiousPlace.size());
        for (int i = 0; i < continiousPlace.size(); i++) {
            index[i] = transitions.size() + continiousPlace[i];
        }
        //Compute convex
        std::set<Point<double>> newPoly;
        std::vector<std::vector<Point<double >>> allPolytopes;
        cout << "amount of flowpipes" << flowpipes.size() << endl;
        for (auto & indexPair: flowpipes) {
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Iterate over flowpipes
            for (auto & set: flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                // Filter points of boxes wether property is satisfied
                if (!points.empty() && points.size() >= 0) {
                    for (auto & point: points) {
                        bool satisfied = true;
                        for (int i = 0; i < value.size(); i++) {
                            auto coordinate = index[i];
                            satisfied &= CompareValuesWithOp(op[i], carl::convert<Number, double>(point.rawCoordinates()[coordinate]), value[i]);
                        }
                        if (satisfied) {
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

        for (int i = 0; i < allPolytopes.size(); i++) {
            cout << "Polytope Nr. " << (i + 1) << endl;
            for (int j = 0; j < allPolytopes[i].size(); j++) {
                for (int k = 0; k < allPolytopes[i][j].rawCoordinates().size(); k++) {
                    cout << allPolytopes[i][j].rawCoordinates()[k] << ", ";
                }
                cout << endl;
            }
        }
        double totalError = 0.0;
        // Compute HPolytope from vertices
        cout << "Amount of Polytopes:" << allPolytopes.size() << endl;
        std::vector<hypro::HPolytope<double>> polytopes;
        for (int i = 0; i < allPolytopes.size(); i++) {
            std::vector<Point<double >> currentPoints = allPolytopes[i];
            hypro::HPolytope<double> polytope = hypro::HPolytope<double> (currentPoints);
            polytopes.push_back(polytope);
        }
        cout << "Polytopes succesful" << endl;
        auto prob = ProbabilityCalculator();
        // Calculate probabilty using HPolytope
        auto res = prob.getProbabilityForUnionOfPolytopesUsingMonteCarlo(polytopes, distributions, 1, 5000, totalError);
        return res;
    }

	std::vector<double> ProbabilityOnFlowpipe::CalculateProbabiltyForPropertyUsingFunction(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributions, double tMax, std::vector<std::function<std::tuple<bool,string>(int, Point<Number>)>> properties) {
		//get Petri net from file

		HybridAutomatonHandler handler(automaton, tMax);
		// Compute flowpipes
		std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes = handler.computeFlowpipes(tMax, 0.01, 5);
		// indizes of all generall transistions
		std::vector<int> transitions = std::vector<int>(automaton -> getInitialGeneral().size());
		std::iota(transitions.begin(), transitions.end(), 0);
		string printout;
		std::vector<double> results;
		std::vector<string> propoutput;
		std::tuple<bool,string> check;
		//Compute convex
		for(auto prop : properties) {
			std::set<Point<double>> newPoly;
			std::vector<std::vector<Point<double >>> allPolytopes;
			for (auto & indexPair: flowpipes) {
				std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
				// Iterate over flowpipes
				for (auto & set: flowpipe) {
					std::vector<Point<Number>> points = set.vertices();
					// Filter points of boxes wether property is satisfied
					if (!points.empty() && points.size() >= 0) {
						for (auto & point: points) {
							check = prop(transitions.size(), point);
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

			for (int i = 0; i < allPolytopes.size(); i++) {
				cout << "Polytope Nr. " << (i + 1) << endl;
				for (int j = 0; j < allPolytopes[i].size(); j++) {
					for (int k = 0; k < allPolytopes[i][j].rawCoordinates().size(); k++) {
						cout << allPolytopes[i][j].rawCoordinates()[k] << ", ";
					}
					cout << endl;
				}
			}

			double totalError = 0.0;
			// Compute HPolytope from vertices
			std::vector<hypro::HPolytope<double>> polytopes;
			for (int i = 0; i < allPolytopes.size(); i++) {
				std::vector<Point<double >> currentPoints = allPolytopes[i];
				hypro::HPolytope<double> polytope = hypro::HPolytope<double> (currentPoints);
				polytopes.push_back(polytope);
			}
			auto probability = ProbabilityCalculator();
			// Calculate probabilty using HPolytope
			auto res = probability.getProbabilityForUnionOfPolytopesUsingMonteCarlo(polytopes, distributions, 1, 5000, totalError);
			propoutput.push_back(std::get<1>(check));
			results.push_back(res);
		}
		PrintResults(propoutput, results);
		return results;
	}

	void ProbabilityOnFlowpipe::PrintResults(std::vector<string> properties, std::vector<double> res) {

    	for(int i = 0; i < properties.size(); i++) {
    		cout << "Probability that ";
    		cout << properties[i];
    		cout << " holds: ";
    		cout << res[i] <<endl;
    	}
    }

    bool ProbabilityOnFlowpipe::CompareValuesWithOp(string op, double val1, double val2) {
        if (op == "<") return val1 < val2;
        if (op == "<=") return val1 <= val2;
        if (op == "==") return val1 == val2;
        if (op == ">") return val1 > val2;
        if (op == ">=") return val1 >= val2;
    }

    bool ProbabilityOnFlowpipe::CheckIfRelevant(Point<double> toCheck, Point<double> ref, std::vector<int> transitions) {
        for (int i: transitions) {
            if (toCheck[i] != ref[i]) return false;
        }
        return true;
    }


}