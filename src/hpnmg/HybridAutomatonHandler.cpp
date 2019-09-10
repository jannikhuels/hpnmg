#include "HybridAutomatonHandler.h"

using namespace std;
using namespace hypro;
using namespace hypro;
typedef mpq_class Number;

namespace hpnmg {


    HybridAutomatonHandler::HybridAutomatonHandler() {}


    HybridAutomaton<Number> HybridAutomatonHandler::convertAutomaton(shared_ptr<SingularAutomaton> singular, double maxTime) {

        singularAutomaton = singular;
        automaton = HybridAutomaton<Number>();

        //get number of variables
        g = singularAutomaton->getInitialGeneral().size();
        x = singularAutomaton->getInitialContinuous().size();
        c = singularAutomaton->getInitialDeterministic().size();
        v = g + x + c;


        vector<shared_ptr<SingularAutomaton::Transition>> originalTransitions;

        //Add all locations to automaton, make a list of outgoing transitions and save index of initial location
        int initial = 0;
        int id = singularAutomaton->getInitialLocation()->getLocationId();

        for (shared_ptr<SingularAutomaton::Location> originalLocation : singularAutomaton->getLocations()) {

            addLocation(originalLocation, maxTime);

            if (originalLocation->getLocationId() == id)
                initial = locations.size() - 1;

            for (shared_ptr<SingularAutomaton::Transition> originalTransition : originalLocation->getOutgoingTransitions())
                originalTransitions.push_back(originalTransition);
        }


        //Add all transitions in list to automaton
        for (shared_ptr<SingularAutomaton::Transition> transition : originalTransitions) {
            addTransition(transition);
        }

        //set initial state from initial location
        setInitialState(initial);

        //add locations and transitions to automaton
        for (int i = 0; i < locations.size(); i++) {
            automaton.addLocation((locations[i]));
        }
        for (int i = 0; i < transitions.size(); i++) {
            automaton.addTransition((transitions[i]));
        }

        return automaton;
    }


    void HybridAutomatonHandler::addLocation(shared_ptr<SingularAutomaton::Location> originalLocation, double maxTime) {

        hypro::Location<Number> newLocation = hypro::Location<Number>();

        cout << "Location no. " << originalLocation->getLocationId();
        // -- Invariants --
        vector<pair<invariantOperator, double>> invariantsX = originalLocation->getInvariantsContinuous();
        vector<pair<invariantOperator, double>> invariantsC = originalLocation->getInvariantsDeterministic();

        vector_t<Number> invariantVec = vector_t<Number>::Zero(2 * v);
        matrix_t<Number> invariantMat = matrix_t<Number>::Zero(2 * v, v);

        //constraints for general variables
        for (int i = 0; i < g; i++) {
            invariantVec(2 * i) = Number(0);
            invariantVec(2 * i + 1) = Number(maxTime);
            invariantMat(2 * i, i) = Number(-1);
            invariantMat(2 * i + 1, i) = Number(1);
            cout << " | Constraint g" << (i + 1) << " in [0," << maxTime << "]";
        }

        //constraints for continuous variables
        for (int i = 0; i < x; i++) {

            invariantVec(2 * g + 2 * i) = Number(0);
            invariantVec(2 * g + 2 * i + 1) = Number(10 * maxTime);
            invariantMat(2 * g + 2 * i, g + i) = Number(-1);
            invariantMat(2 * g + 2 * i + 1, g + i) = Number(1);

            if (invariantsX[i].first != UNLIMITED) {

                if (invariantsX[i].first == GREATER_EQUAL) {
                    invariantVec(2 * g + 2 * i) = Number(-1 * carl::rationalize<Number>(invariantsX[i].second));
                } else {
                    invariantVec(2 * g + 2 * i + 1) = Number(carl::rationalize<Number>(invariantsX[i].second));
                }

                cout << " | Constraint x" << (i + 1) << " in [" << invariantVec(2 * g + 2 * i) << "," << invariantVec(2 * g + 2 * i + 1) << "]";
            }
        }

        //constraints for deterministic clocks
        for (int i = 0; i < c; i++) {
            invariantVec(2 * g + 2 * x + 2 * i) = Number(0);
            invariantVec(2 * g + 2 * x + 2 * i + 1) = Number(maxTime);
            invariantMat(2 * g + 2 * x + 2 * i, g + x + i) = Number(-1);
            invariantMat(2 * g + 2 * x + 2 * i + 1, g + x + i) = Number(1);

            if (invariantsC[i].first != UNLIMITED) {

                if (invariantsC[i].first == GREATER_EQUAL)
                    invariantVec(2 * g + 2 * x + 2 * i) = Number(-1 * carl::rationalize<Number>(invariantsC[i].second));
                else
                    invariantVec(2 * g + 2 * x + 2 * i + 1) = Number(carl::rationalize<Number>(invariantsC[i].second));

           }

        }




        Condition<Number> inv(invariantMat, invariantVec);
        newLocation.setInvariant(inv);


        // -- Activities --
        // setup flow matrix (add an artificial dimension to cope with constants).
        matrix_t<Number> flowMatrix = matrix_t<Number>::Zero(v + 1, v + 1);

        //flow for general transition evolution
        vector<bool> actG = originalLocation->getActivitiesGeneral();
        for (int i = 0; i < actG.size(); i++) {
            if (actG[i]) {
                flowMatrix(i, v) = Number(1);
                cout << " | Flow g" << (i + 1) << "'=" << 1;
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

        locations.push_back(newLocation);
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
            guardVec(0) = Number(-1 * carl::rationalize<Number>(originalTransition->getValueGuardCompare()));
            guardMat(0, varIndex) = Number(-1);

            cout << " | Guard " << type << "(" << varIndex << ") >=" << -1 * guardVec(0);

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
        newTransition.setSource(&locations[source]);
        newTransition.setTarget(&locations[target]);

        transitions.push_back(newTransition);
        locations[source].addTransition(&transitions[transitions.size() - 1]);
    }


    void HybridAutomatonHandler::setInitialState(int initial) {

        //get initial values
        vector<double> initG = singularAutomaton->getInitialGeneral();
        vector<double> initX = singularAutomaton->getInitialContinuous();
        vector<double> initC = singularAutomaton->getInitialDeterministic();

        // create Box holding the initial set.

        matrix_t<Number> boxMat = matrix_t<Number>::Zero(2 * v, v);
        vector_t<Number> boxVec = vector_t<Number>::Zero(2 * v);

        cout << "Initial state index: " << initial;

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

        cout << endl;

        // create initial state.
        State_t<Number> initialState;
        initialState.setLocation(&locations[initial]);
        initialState.setSet(ConstraintSet<Number>(boxMat, boxVec));
        automaton.addInitialState(initialState);
        automatonExists = true;
    }


    std::vector<std::pair<unsigned, hypro::reachability::flowpipe_t<Number>>> HybridAutomatonHandler::computeFlowpipes(double maxTime, double timestep, int jumpDepth) {

            std::vector<std::pair<unsigned, hypro::reachability::flowpipe_t<Number>>> flowpipes ;
           if (!automatonExists) {
               cout << "No automaton exists to compute flow pipes for. Empty vector returned.";
               return flowpipes;
           }

           typedef HPolytope<Number> Representation;

           // instanciate reachability analysis algorithm
           reachability::Reach<Number, reachability::ReachSettings> reacher(automaton);
           ReachabilitySettings settings = reacher.settings();
           settings.timeStep = carl::convert<double, Number>(timestep);
           settings.timeBound = Number(maxTime);
           settings.jumpDepth = jumpDepth;
           reacher.setSettings(settings);
           reacher.setRepresentationType(Representation::type());

           // perform reachability analysis.
           flowpipes = reacher.computeForwardReachability();

           flowpipesComputed = true;

           cout << "Points: " << endl;

        for (auto &indexPair : flowpipes) {
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for (auto &set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if (!points.empty() && points.size() >= 0) {
                    for (auto &point : points) {
                        cout << point << endl;
                    }
                    points.clear();
                }
            }
        }

           return flowpipes;
       }


    void HybridAutomatonHandler::plotTex(string outputfile, std::vector<std::pair<unsigned, hypro::reachability::flowpipe_t<Number>>> flowpipes) {

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