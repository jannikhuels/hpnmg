#include "HybridAutomatonHandler.h"
#include <ProbabilityCalculator.h>

using namespace std;
using namespace hypro;
typedef mpq_class Number;

namespace hpnmg {

    HybridAutomatonHandler::HybridAutomatonHandler(shared_ptr<SingularAutomaton> singular, double maxTime):singularAutomaton(singular)  {//, automaton(std::make_unique<HybridAutomaton<Number>>()) {

        //get number of variables
        g = singularAutomaton->getInitialGeneral().size();
        x = singularAutomaton->getInitialContinuous().size();
        c = singularAutomaton->getInitialDeterministic().size();
        v = g + x + c;

        vector<shared_ptr<SingularAutomaton::Transition>> originalTransitions;

        //Add all locations to automaton, make a list of outgoing transitions and save index of initial location
        vector<int> initial;
        for (shared_ptr<SingularAutomaton::Location> originalLocation : singularAutomaton->getLocations()) {

            addLocation(originalLocation, maxTime);

            if (singular->isInitialLocation(originalLocation))
                initial.push_back(locations.size() - 1);


            for (shared_ptr<SingularAutomaton::Transition> originalTransition : originalLocation->getOutgoingTransitions())
                originalTransitions.push_back(originalTransition);
        }

        this->automaton = HybridAutomaton<Number>();

        //Add all transitions in list to automaton
        for (shared_ptr<SingularAutomaton::Transition> transition : originalTransitions) {
            addTransition(transition);
        }

        //set initial state from initial location
        setInitialState(initial);

        //add locations and transitions to automaton
        for (int i = 0; i < locations.size(); i++) {
            this->automaton.addLocation(move(locations[i]));
        }
    }


    void HybridAutomatonHandler::addLocation(shared_ptr<SingularAutomaton::Location> originalLocation, double maxTime) {

        hypro::Location<Number> newLocation = hypro::Location<Number>();

        cout << "Location no. " << originalLocation->getLocationId();
        // -- Invariants --
        vector<pair<invariantOperator, double>> invariantsX = originalLocation->getInvariantsContinuous();
        vector<pair<invariantOperator, double>> invariantsC = originalLocation->getInvariantsDeterministic();


        int countX = 0;
        for (int i = 0; i < invariantsX.size(); i++) {
            if (invariantsX[i].first != UNLIMITED)
                countX++;
        }

        int countC = 0;
        for (int i = 0; i < invariantsC.size(); i++) {
            if (invariantsC[i].first != UNLIMITED)
                countC++;
        }

        if (countX + countC > 0) {

            vector_t <Number> invariantVec = vector_t<Number>::Zero(countX + countC);
            matrix_t <Number> invariantMat = matrix_t<Number>::Zero(countX + countC, v);


            //constraints for continuous variables
            countX = 0;
            for (int i = 0; i < x; i++) {

                if (invariantsX[i].first != UNLIMITED) {

                    if (invariantsX[i].first == GREATER_EQUAL) {
                        invariantMat(countX, g + i) = Number(-1);
                        invariantVec(countX) = Number(-1 * carl::rationalize<Number>(invariantsX[i].second));
                        cout << " | Constraint x" << (i + 1) << ">=" << invariantVec(countX) * (-1);
                    } else {
                        invariantMat(countX, g + i) = Number(1);
                        invariantVec(countX) = Number(carl::rationalize<Number>(invariantsX[i].second));
                        cout << " | Constraint x" << (i + 1) << "<=" << invariantVec(countX);
                    }
                    countX++;
                }
            }

            //constraints for deterministic clocks
            countC = 0;
            for (int i = 0; i < c; i++) {

                if (invariantsC[i].first != UNLIMITED) {

                    if (invariantsC[i].first == GREATER_EQUAL) {
                        invariantMat(countX + countC, g + x + i) = Number(-1);
                        invariantVec(countX + countC) = Number(-1 * carl::rationalize<Number>(invariantsC[i].second));
                        cout << " | Constraint c" << (i + 1) << ">=" << invariantVec(countX + countC) * (-1);
                    } else {
                        invariantMat(countX + countC, g + x + i) = Number(1);
                        invariantVec(countX + countC) = Number(carl::rationalize<Number>(invariantsC[i].second));
                        cout << " | Constraint c" << (i + 1) << "<=" << invariantVec(countX + countC);
                    }
                    countC++;
                }

            }


            Condition <Number> inv(invariantMat, invariantVec);
            newLocation.setInvariant(inv);
        }


        // -- Activities --
        // setup flow matrix (add an artificial dimension to cope with constants).
        matrix_t<Number> flowMatrix = matrix_t<Number>::Zero(v + 1, v + 1);

        //flow for general transition evolution
        vector<short int> actG = originalLocation->getActivitiesGeneral();
        for (int i = 0; i < actG.size(); i++) {
            if (actG[i] != 0) {
                flowMatrix(i, v) = Number(carl::rationalize<Number>(actG[i]));
                cout << " | Flow g" << (i + 1) << "'=" << actG[i];
            }
        }

        //flow for continuous variables
        vector<double> actX = originalLocation->getActivitiesContinuous();
        for (int i = 0; i < actX.size(); i++) {
            if (actX[i] != 0.0) {
                flowMatrix(g + i, v) = Number(carl::rationalize<Number>(actX[i]));
                cout << " | Flow x" << (i + 1) << "'=" << actX[i];
            }
        }

        //flow for deterministic clocks
        vector<bool> actC = originalLocation->getActivitiesDeterministic();
        for (int i = 0; i < actC.size(); i++) {
            if (actC[i]) {
                flowMatrix(g + x + i, v) = Number(1);
                cout << " | Flow c" << (i + 1) << "'=" << 1;
            }
        }

        cout << endl;

        newLocation.setFlow(flowMatrix);

        locations.push_back(make_unique<hypro::Location<Number>>(newLocation));
    }


    void HybridAutomatonHandler::addTransition(shared_ptr<SingularAutomaton::Transition> originalTransition) {


        hypro::Transition<Number> newTransition = hypro::Transition<Number>();
        newTransition.setAggregation(Aggregation::parallelotopeAgg);

        //Find connected locations indices
        int index = 0;
        int source = 0;
        int target = 0;
        int sourceId = originalTransition->getPredecessorLocation()->getLocationId();
        int targetId = originalTransition->getSuccessorLocation()->getLocationId();

        for (shared_ptr<SingularAutomaton::Location> originalLocation : singularAutomaton->getLocations()) {

            if (originalLocation->getLocationId() == sourceId)
                source = index;

            if (originalLocation->getLocationId() == targetId)
                target = index;

            index++;
        }

        cout << "Transition from " << sourceId << "(" << source << ")" << " to " << targetId << "(" << target << ")";


        EventType type = originalTransition->getType();
        int varIndex = originalTransition->getVariableIndex();

        // -- Guard -- (only for x and c)
        if (type == Continuous || type == Timed) {

            varIndex += g;
            if (type == Timed)
                varIndex += x;

            Condition<Number> guard;
            matrix_t<Number> guardMat = matrix_t<Number>::Zero(1, v);
            vector_t<Number> guardVec = vector_t<Number>(1);

            if (type == Continuous && originalTransition->getPredecessorLocation()->getActivitiesContinuous()[originalTransition->getVariableIndex()] < 0.0) {
            //if derivative for continuous variable in source location is negative, guard for lower equal, otherwise guard for greater equal
                guardVec(0) = Number(carl::rationalize<Number>(originalTransition->getValueGuardCompare()));
                guardMat(0, varIndex) = Number(1);
                cout << " | Guard " << type << "(" << varIndex << ") <=" << guardVec(0);
            } else {
                guardVec(0) = Number(-1 * carl::rationalize<Number>(originalTransition->getValueGuardCompare()));
                guardMat(0, varIndex) = Number(-1);
                cout << " | Guard " << type << "(" << varIndex << ") >=" << -1 * guardVec(0);
            }

            guard.setMatrix(guardMat);
            guard.setVector(guardVec);

            newTransition.setGuard(guard);
        }


        // -- Reset --

        Reset<Number> reset;
        vector_t<Number> constantReset = vector_t<Number>::Zero(v);

        matrix_t<Number> linearReset = matrix_t<Number>::Identity(v, v);

        if (type == General || type == Timed) {
            linearReset(varIndex, varIndex) = Number(0);

            cout << " | Reset " << type << "(" << varIndex << ") =" << 0;
        }

        cout << endl;

        reset.setVector(constantReset);
        reset.setMatrix(linearReset);
        newTransition.setReset(reset);

        // -- Connecting locations --
        newTransition.setSource(locations[source]);
        newTransition.setTarget(locations[target]);

        //transitions.push_back(newTransition);
        locations[source]->addTransition(std::make_unique<hypro::Transition<Number>>(newTransition));
    }


    void HybridAutomatonHandler::setInitialState(vector<int> initial) {

        //get initial values
        vector<double> initG = singularAutomaton->getInitialGeneral();
        vector<double> initX = singularAutomaton->getInitialContinuous();
        vector<double> initC = singularAutomaton->getInitialDeterministic();

        // create Box holding the initial set.

        matrix_t<Number> boxMat = matrix_t<Number>::Zero(2 * v, v);
        vector_t<Number> boxVec = vector_t<Number>::Zero(2 * v);

        cout << "Initial state: ";

        for (int i = 0; i < initG.size(); i++) {
            boxVec(i) = Number(-1 * carl::rationalize<Number>(initG[i]));
            boxVec(i + 1) = Number(carl::rationalize<Number>(initG[i]));
            boxMat(2 * i, i) = Number(-1);
            boxMat(2 * i + 1, i) = Number(1);
            cout << " | g" << i + 1 << " = " << initG[i];
        }

        for (int i = 0; i < initX.size(); i++) {
            boxVec(2 * g + i) = Number(-1 * carl::rationalize<Number>(initX[i]));
            boxVec(2 * g + i + 1) = Number(carl::rationalize<Number>(initX[i]));
            boxMat(2 * g + 2 * i, g + i) = Number(-1);
            boxMat(2 * g + 2 * i + 1, g + i) = Number(1);
            cout << " | x" << i  + 1 << " = " << initX[i];
        }

        for (int i = 0; i < initC.size(); i++) {
            boxVec(2 * g + 2 * x + i) = Number(-1 * carl::rationalize<Number>(initC[i]));
            boxVec(2 * g + 2 * x + i + 1) = Number(carl::rationalize<Number>(initC[i]));
            boxMat(2 * g + 2 * x + 2 * i, g + x + i) = Number(-1);
            boxMat(2 * g + 2 * x + 2 * i + 1, g + x + i) = Number(1);
            cout << " | c" << i  + 1 << " = " << initC[i];
        }

        // create initial states.
        for (int index : initial){
            automaton.addInitialState(locations[index].get(), Condition<Number>(boxMat, boxVec));
            cout << " | location index " << index;
        }

        cout << endl;
    }


    std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> HybridAutomatonHandler::computeFlowpipes(double maxTime, double timestep, int jumpDepth) {

           std::vector<std::pair<unsigned, flowpipe_t>> flowpipes;

           typedef Box<Number> Representation;

           // instanciate reachability analysis algorithm
           reachability::Reach<Number, reachability::ReachSettings, hypro::State_t<Number>> reacher(this->automaton);
           ReachabilitySettings settings = reacher.settings();
           settings.timeStep = carl::convert<double, Number>(timestep);
           settings.timeBound = Number(maxTime);
           settings.jumpDepth = jumpDepth;
           reacher.setSettings(settings);
           reacher.setRepresentationType(Representation::type());

           // perform reachability analysis.
           flowpipes = reacher.computeForwardReachability();

           flowpipesComputed = true;

           cout << endl;

        for (auto &indexPair : flowpipes) {
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for (auto &set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if (!points.empty() && points.size() >= 0) {
                    for (auto &point : points) {

                        for (int i = 0; i < point.rawCoordinates().size(); i++){
                            auto coordinate = point.rawCoordinates()[i];
                            //cout << carl::convert<Number, double>(coordinate) << ", ";
                        }
                        //cout << endl;
                    }
                    points.clear();
                }
            }
        }

           return flowpipes;
       }


    void HybridAutomatonHandler::plotTex(string outputfile, std::vector<std::pair<unsigned, HybridAutomatonHandler::flowpipe_t>> flowpipes) {

        if (!flowpipesComputed) {
            cout << "No flowpipes exist to plot.";
            return;
        }

        // plot flowpipes.
        Plotter<Number> &plotter = Plotter<Number>::getInstance();
        plotter.setFilename(outputfile);
        for (auto &indexPair : flowpipes) {
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for (auto &set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if (!points.empty() && points.size() >= 0) {
                    for (auto &point : points) {
                        //cout << point;
                        point.reduceDimension(2);
                    }
                    plotter.addObject(points);
                    points.clear();
                }
            }
        }
        // write output.
        plotter.plotTex();
    }


}