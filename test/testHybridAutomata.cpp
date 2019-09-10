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
#include "gtest/gtest.h"
#include "PLTWriter.h"
#include <chrono>

#include "config.h"
#include "datastructures/HybridAutomaton/LocationManager.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "datastructures/Point.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"
#include "parser/antlr4-flowstar/ParserWrapper.h"


using namespace hpnmg;
using namespace std;



unsigned long getNodes(const shared_ptr<ParametricLocationTree>& plt, const ParametricLocationTree::Node& node) {
    vector<ParametricLocationTree::Node> children = plt->getChildNodes(node);
    unsigned long nodes = children.size();
    for (const ParametricLocationTree::Node& child : children)
        nodes += getNodes(plt, child);
    return nodes;
}


unsigned long getNumberOfLocations(const shared_ptr<SingularAutomaton>& automaton) {
    return automaton->getLocations().size();
}

unsigned long getNumberOfEdges(const shared_ptr<SingularAutomaton>& automaton) {
    unsigned long numberOfEdges = 0;
    for(const shared_ptr<SingularAutomaton::Location>& loc : automaton->getLocations()) {
        numberOfEdges += loc->getOutgoingTransitions().size();
    }
    return numberOfEdges;
}






TEST(HybridAutomaton, example) {


    ReadHybridPetrinet reader;
    PLTWriter PLTwriter;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;


// setup
    string filePath = "../../test/testfiles/example.xml";
    double tauMax = 10.0;

// read
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

    auto parser = new ParseHybridPetrinet();
    auto plt0 = parser->parseHybridPetrinet(hybridPetriNet, tauMax);

// transform
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tauMax));
    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);

// print
    unsigned long p = getNodes(plt, plt->getRootNode()) + 1;
    unsigned long l = getNumberOfLocations(automaton);
    unsigned long e = getNumberOfEdges(automaton);

    cout << "t_max=" << tauMax << ", #PL=" << p << ", #Loc=" << l << ", #Edge=" << e << "\n" << endl;

// write
    PLTwriter.writePLT(plt, tauMax, "plt_example_10");
    automatonWriter.writeAutomaton(automaton, "example_10");

}




TEST(HybridAutomaton, exampleAutomaton) {

    using namespace hypro;

    // typedefs for simplification.
    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    // create the discrete structure of the automaton and the automaton itself.
    unique_ptr<Location<Number>> loc1 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc3 = make_unique<Location<Number>>();
    unique_ptr<Transition<Number>> trans = make_unique<Transition<Number>>();
    HybridAutomaton<Number> exampleAutomaton = HybridAutomaton<Number>();


    // location L1 parameter setting.

    // creation of the invariant.
    vector_t<Number> invariantVec = vector_t<Number>(1);
    matrix_t<Number> invariantMat = matrix_t<Number>(1,3);
    invariantVec(0) = Number(5);
    invariantMat(0,0) = Number(0);
    invariantMat(0,1) = Number(1);
    invariantMat(0,2) = Number(0);

    Condition<Number> inv(invariantMat,invariantVec);
    loc1->setInvariant(inv);


    // setup flow matrix (4x4, we add an artificial dimension to cope with constants).
    matrix_t<Number> flowMatrix = matrix_t<Number>(4,4);
    flowMatrix(0,0) = Number(0);
    flowMatrix(0,1) = Number(0);
    flowMatrix(0,2) = Number(0);
    flowMatrix(0,3) = Number(1);
    flowMatrix(1,0) = Number(0);
    flowMatrix(1,1) = Number(0);
    flowMatrix(1,2) = Number(0);
    flowMatrix(1,3) = Number(1);
    flowMatrix(2,0) = Number(0);
    flowMatrix(2,1) = Number(0);
    flowMatrix(2,2) = Number(0);
    flowMatrix(2,3) = Number(1);
    flowMatrix(3,0) = Number(0);
    flowMatrix(3,1) = Number(0);
    flowMatrix(3,2) = Number(0);
    flowMatrix(3,3) = Number(0);
    loc1->setFlow(flowMatrix);


    // location L3 parameter setting.

    // creation of the invariant.
    vector_t<Number> invariantVec3 = vector_t<Number>(1);
    matrix_t<Number> invariantMat3 = matrix_t<Number>(1,3);
    invariantVec3(0) = Number(10);
    invariantMat3(0,0) = Number(1);
    invariantMat3(0,1) = Number(0);
    invariantMat3(0,2) = Number(0);
    //TODO bei mehreren Conditions, Vector übergeben
    Condition<Number> inv3(invariantMat3,invariantVec3);
    loc3->setInvariant(inv3);


    // setup flow matrix (4x4, we add an artificial dimension to cope with constants).
    matrix_t<Number> flowMatrix3 = matrix_t<Number>(4,4);
    flowMatrix3(0,0) = Number(0);
    flowMatrix3(0,1) = Number(0);
    flowMatrix3(0,2) = Number(0);
    flowMatrix3(0,3) = Number(2);
    flowMatrix3(1,0) = Number(0);
    flowMatrix3(1,1) = Number(0);
    flowMatrix3(1,2) = Number(0);
    flowMatrix3(1,3) = Number(0);
    flowMatrix3(2,0) = Number(0);
    flowMatrix3(2,1) = Number(0);
    flowMatrix3(2,2) = Number(0);
    flowMatrix3(2,3) = Number(1);
    flowMatrix3(3,0) = Number(0);
    flowMatrix3(3,1) = Number(0);
    flowMatrix3(3,2) = Number(0);
    flowMatrix3(3,3) = Number(0);
    loc3->setFlow(flowMatrix3);


    // setup of the transition.

    // guard
    Condition<Number> guard;
    matrix_t<Number> guardMat = matrix_t<Number>(1,3);
    vector_t<Number> guardVec = vector_t<Number>(1);
    guardVec(0) = Number(-5);
    guardMat(0,0) = Number(0);
    guardMat(0,1) = Number(-1);
    guardMat(0,2) = Number(0);
    guard.setMatrix(guardMat);
    guard.setVector(guardVec);

    // reset function
    Reset<Number> reset;
    vector_t<Number> constantReset = vector_t<Number>(3);
    matrix_t<Number> linearReset = matrix_t<Number>(3,3);
    constantReset(0) = Number(0);
    constantReset(1) = Number(0);
    constantReset(2) = Number(0);
    linearReset(0,0) = Number(1);
    linearReset(0,1) = Number(0);
    linearReset(0,2) = Number(0);
    linearReset(1,0) = Number(0);
    linearReset(1,1) = Number(0);
    linearReset(1,2) = Number(0);
    linearReset(2,0) = Number(0);
    linearReset(2,1) = Number(0);
    linearReset(2,2) = Number(1);
    reset.setVector(constantReset);
    reset.setMatrix(linearReset);

    // setup transition
    trans->setAggregation(Aggregation::parallelotopeAgg);
    trans->setGuard(guard);
    trans->setSource(loc1);
    trans->setTarget(loc3);
    trans->setReset(reset);

    // add defined location and transition to the automaton.
    loc1->addTransition(trans.get());

    // create Box holding the initial set.
    matrix_t<Number> boxMat = matrix_t<Number>(6,3);
    vector_t<Number> boxVec = vector_t<Number>(6);
    boxVec(0) = Number(0);
    boxVec(1) = Number(0);
    boxVec(2) = Number(0);
    boxVec(3) = Number(0);
    boxVec(4) = Number(0);
    boxVec(5) = Number(0);
    boxMat(0,0) = Number(1);
    boxMat(0,1) = Number(0);
    boxMat(0,2) = Number(0);
    boxMat(1,0) = Number(-1);
    boxMat(1,1) = Number(0);
    boxMat(1,2) = Number(0);
    boxMat(2,0) = Number(0);
    boxMat(2,1) = Number(1);
    boxMat(2,2) = Number(0);
    boxMat(3,0) = Number(0);
    boxMat(3,1) = Number(-1);
    boxMat(3,2) = Number(0);
    boxMat(4,0) = Number(0);
    boxMat(4,1) = Number(0);
    boxMat(4,2) = Number(1);
    boxMat(5,0) = Number(0);
    boxMat(5,1) = Number(0);
    boxMat(5,2) = Number(-1);

    // create initial state.
    State_t<Number> initialState;
    initialState.setLocation(loc1.get());
    initialState.setSet(ConstraintSet<Number>(boxMat, boxVec));
    exampleAutomaton.addInitialState(initialState);

    exampleAutomaton.addLocation(move(loc1));
    exampleAutomaton.addLocation(move(loc3));
    exampleAutomaton.addTransition(move(trans));

    // instanciate reachability analysis algorithm.
    reachability::Reach<Number, reachability::ReachSettings> reacher(exampleAutomaton);
    ReachabilitySettings settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(10); //time bound
    settings.jumpDepth = 3;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    // perform reachability analysis.
    auto flowpipeIndices = reacher.computeForwardReachability();

    // plot flowpipes.
        Plotter<Number>& plotter = Plotter<Number>::getInstance();
        plotter.setFilename("simpleexample");
        for(auto& indexPair : flowpipeIndices){
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for(auto& set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if(!points.empty() && points.size() > 2) {
                    for(auto& point : points) {
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


TEST(HybridAutomaton, converter) {

    using namespace hypro;

    ReadHybridPetrinet reader;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;

// setup
    string filePath = "../../test/testfiles/example.xml";
    double tMax = 15.0;

// read
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);



// transform
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));

    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    auto writer = new PLTWriter();
    writer->writePLT(plt, tMax);

    shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
    HybridAutomatonHandler handler ;
    HybridAutomaton<Number> exampleAutomaton = handler.convertAutomaton(automaton, tMax);

//compute flowpipes
    auto flowpipes = handler.computeFlowpipes(tMax, 0.1, 5);

    handler.plotTex("example", flowpipes);
}


TEST(HybridAutomaton, Bouncingball) {

    using namespace hypro;

    // typedefs for simplification.
    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    // create the discrete structure of the automaton and the automaton itself.
    unique_ptr<Location<Number>> loc1 = make_unique<Location<Number>>();
    unique_ptr<Transition<Number>> trans = make_unique<Transition<Number>>();
    HybridAutomaton<Number> bBallAutomaton = HybridAutomaton<Number>();

    // matrix defining the flow (note: 3rd dimension for constant parts).
    matrix_t<Number> flowMatrix = matrix_t<Number>(3,3);


    // location parameter setting.

    // creation of the invariant.
    matrix_t<Number> invariantMat = matrix_t<Number>(1,2);
    vector_t<Number> invariantVec = vector_t<Number>(1);
    invariantVec(0) = Number(0);
    invariantMat(0,0) = Number(-1);
    invariantMat(0,1) = Number(0);
    Condition<Number> inv(invariantMat,invariantVec);
    loc1->setInvariant(inv);

    // setup flow matrix (3x3, we add an artificial dimension to cope with constants).
    flowMatrix(0,0) = Number(0);
    flowMatrix(0,1) = Number(1);
    flowMatrix(0,2) = Number(0);
    flowMatrix(1,0) = Number(0);
    flowMatrix(1,1) = Number(0);
    flowMatrix(1,2) = Number(carl::rationalize<Number>(-9.81));
    flowMatrix(2,0) = Number(0);
    flowMatrix(2,1) = Number(0);
    flowMatrix(2,2) = Number(0);
    loc1->setFlow(flowMatrix);


    // setup of the transition.

    // guard
    Condition<Number> guard;
    matrix_t<Number> guardMat = matrix_t<Number>(3,2);
    vector_t<Number> guardVec = vector_t<Number>(3);
    guardVec(0) = Number(0);
    guardVec(1) = Number(0);
    guardVec(2) = Number(0);
    guardMat(0,0) = Number(1);
    guardMat(0,1) = Number(0);
    guardMat(1,0) = Number(-1);
    guardMat(1,1) = Number(0);
    guardMat(2,0) = Number(0);
    guardMat(2,1) = Number(1);
    guard.setMatrix(guardMat);
    guard.setVector(guardVec);

    // reset function
    Reset<Number> reset;
    vector_t<Number> constantReset = vector_t<Number>(2,1);
    matrix_t<Number> linearReset = matrix_t<Number>(2,2);
    constantReset(0) = Number(0);
    constantReset(1) = Number(0);
    linearReset(0,0) = Number(1);
    linearReset(0,1) = Number(0);
    linearReset(1,0) = Number(0);
    linearReset(1,1) = Number(carl::rationalize<Number>(-0.9)); //c
    reset.setVector(constantReset);
    reset.setMatrix(linearReset);

    // setup transition
    trans->setAggregation(Aggregation::parallelotopeAgg);
    trans->setGuard(guard);
    trans->setSource(loc1);
    trans->setTarget(loc1);
    trans->setReset(reset);

    // add defined location and transition to the automaton.
    loc1->addTransition(trans.get());


    // create Box holding the initial set.

    matrix_t<Number> boxMat = matrix_t<Number>(4,2);
    vector_t<Number> boxVec = vector_t<Number>(4);
    boxVec(0) = Number(carl::rationalize<Number>(10.2));
    boxVec(1) = Number(-10);
    boxVec(2) = Number(0);
    boxVec(3) = Number(0);
    boxMat(0,0) = Number(1);
    boxMat(0,1) = Number(0);
    boxMat(1,0) = Number(-1);
    boxMat(1,1) = Number(0);
    boxMat(2,0) = Number(0);
    boxMat(2,1) = Number(1);
    boxMat(3,0) = Number(0);
    boxMat(3,1) = Number(-1);

    // create initial state.
    State_t<Number> initialState;
    initialState.setLocation(loc1.get());
    initialState.setSet(ConstraintSet<Number>(boxMat, boxVec));
    //bBallAutomaton.addInitialState(loc1.get(), Condition<Number>(boxMat,boxVec));
    bBallAutomaton.addInitialState(initialState);
    bBallAutomaton.addLocation(move(loc1));
    bBallAutomaton.addTransition(move(trans));


    // instanciate reachability analysis algorithm.
    reachability::Reach<Number, reachability::ReachSettings> reacher(bBallAutomaton);
    ReachabilitySettings settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(3); //time bound
    settings.jumpDepth = 3;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    // perform reachability analysis.
    auto flowpipeIndices = reacher.computeForwardReachability();

    // plot flowpipes.
        Plotter<Number>& plotter = Plotter<Number>::getInstance();
        plotter.setFilename("bouncingBall");
        for(auto& indexPair : flowpipeIndices){
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for(auto& set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if(!points.empty() && points.size() > 2) {
                    for(auto& point : points) {
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


TEST(HybridAutomaton, FlowParser){

    using namespace hypro;

    // typedefs for simplification.
    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    std::pair<hypro::HybridAutomaton<Number>, hypro::ReachabilitySettings> ha = std::move(hypro::parseFlowstarFile<Number>("../../test/testfiles/exampleeasier.model"));


    reachability::Reach<Number, reachability::ReachSettings> reacher(ha.first, ha.second);


    ReachabilitySettings settings = reacher.settings();
    settings.timeBound = Number(15); //time bound
    settings.jumpDepth = 5;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    auto flowpipeIndices = reacher.computeForwardReachability();

    // plot flowpipes.
        Plotter<Number>& plotter = Plotter<Number>::getInstance();
        plotter.setFilename("example");
        for(auto& indexPair : flowpipeIndices){
            std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
            // Plot single flowpipe
            for(auto& set : flowpipe) {
                std::vector<Point<Number>> points = set.vertices();
                if(!points.empty() && points.size() >= 0) {
                    for(auto& point : points) {
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