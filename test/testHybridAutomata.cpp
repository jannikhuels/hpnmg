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
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "datastructures/Point.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"
#include "parser/antlr4-flowstar/ParserWrapper.h"


#include <datastructures/reachability/SettingsProvider.h>
#include <datastructures/reachability/ReachTree.h>
#include <datastructures/reachability/Task.h>
#include <datastructures/reachability/workQueue/WorkQueueManager.h>
#include <datastructures/reachability/timing/EventTimingProvider.h>
#include <algorithms/reachability/workers/ContextBasedReachabilityWorker.h>


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
    unique_ptr<hypro::Transition<Number>> trans = make_unique<hypro::Transition<Number>>();
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
    loc1->addTransition(move(trans));

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
    exampleAutomaton.addInitialState(loc1.get(), Condition<Number>(boxMat, boxVec));

    exampleAutomaton.addLocation(move(loc1));
    exampleAutomaton.addLocation(move(loc3));

    // instanciate reachability analysis algorithm.
    hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>> reacher(exampleAutomaton);
    ReachabilitySettings settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(10); //time bound
    settings.jumpDepth = 5;
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
    typedef HPolytope<Number> Representation;

    ReadHybridPetrinet reader;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;

// setup
    string filePath = "../../test/testfiles/example.xml";
    double tMax = 10.0;

// read
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);


// transform
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));

    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    auto writer = new PLTWriter();
    writer->writePLT(plt, tMax);

    shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
    HybridAutomatonHandler handler(automaton, tMax);

//compute flowpipes
    auto flowpipes = handler.computeFlowpipes(tMax, 0.1, 5);

    handler.plotTex("example", flowpipes);

}




TEST(HybridAutomaton, MinimalExample) {

    using namespace hypro;

    // typedefs for simplification.
    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    // create the discrete structure of the automaton and the automaton itself.
    unique_ptr<Location<Number>> loc1 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc2 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc3 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc4 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc5 = make_unique<Location<Number>>();
    unique_ptr<Location<Number>> loc6 = make_unique<Location<Number>>();

    unique_ptr<hypro::Transition<Number>> trans12 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans13 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans24 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans25 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans34 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans36 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans54 = make_unique<hypro::Transition<Number>>();
    unique_ptr<hypro::Transition<Number>> trans64 = make_unique<hypro::Transition<Number>>();
    HybridAutomaton<Number> minimalAutomaton = HybridAutomaton<Number>();


    //LOC1
    vector_t<Number> invariantVec1 = vector_t<Number>::Zero(1);
    matrix_t<Number> invariantMat1 = matrix_t<Number>::Zero(1,3);
    invariantVec1(0) = Number(5);
    invariantMat1(0,2) = Number(1); //c <= 5

    Condition<Number> inv1(invariantMat1,invariantVec1);
    loc1->setInvariant(inv1);

    matrix_t<Number> flowMatrix1 = matrix_t<Number>::Zero(4,4);
    flowMatrix1(0,3) = Number(1); //g
    flowMatrix1(1,3) = Number(1); //x
    flowMatrix1(2,3) = Number(1); //c
    loc1->setFlow(flowMatrix1);


    //LOC2
    vector_t<Number> invariantVec2 = vector_t<Number>::Zero(2);
    matrix_t<Number> invariantMat2 = matrix_t<Number>::Zero(2, 3);
    invariantVec2(0) = Number(0);
    invariantVec2(1) = Number(5);
    invariantMat2(0, 1) = Number(-1);
    invariantMat2(1, 2) = Number(1); //x >= 0 10 && c <= 5

    Condition<Number> inv2(invariantMat2, invariantVec2);
    loc2->setInvariant(inv2);

    matrix_t<Number> flowMatrix2 = matrix_t<Number>::Zero(4,4);

    flowMatrix2(0,3) = Number(0); //g
    flowMatrix2(1,3) = Number(-2); //x
    flowMatrix2(2,3) = Number(1); //c
    loc2->setFlow(flowMatrix2);


    //LOC3
    vector_t<Number> invariantVec3 = vector_t<Number>::Zero(1);
    matrix_t<Number> invariantMat3 = matrix_t<Number>::Zero(1, 3);
    invariantVec3(0) = Number(10);
    invariantMat3(0, 1) = Number(1);//x <= 10

    Condition<Number> inv3(invariantMat3, invariantVec3);
    loc3->setInvariant(inv3);

    matrix_t<Number> flowMatrix3 = matrix_t<Number>::Zero(4,4);

    flowMatrix3(0,3) = Number(1); //g
    flowMatrix3(1,3) = Number(2); //x
    flowMatrix3(2,3) = Number(0); //c
    loc3->setFlow(flowMatrix3);


    //LOC4
    matrix_t<Number> flowMatrix4 = matrix_t<Number>::Zero(4,4);

    flowMatrix4(0,3) = Number(0); //g
    flowMatrix4(1,3) = Number(0); //x
    flowMatrix4(2,3) = Number(0); //c
    loc4->setFlow(flowMatrix4);


    //LOC5
    vector_t<Number> invariantVec5 = vector_t<Number>::Zero(1);
    matrix_t<Number> invariantMat5 = matrix_t<Number>::Zero(1, 3);
    invariantVec5(0) = Number(5);
    invariantMat5(0,2) = Number(1); //c <= 5

    Condition<Number> inv5(invariantMat5, invariantVec5);
    loc5->setInvariant(inv5);

    matrix_t<Number> flowMatrix5 = matrix_t<Number>::Zero(4,4);

    flowMatrix5(0,3) = Number(0); //g
    flowMatrix5(1,3) = Number(0); //x
    flowMatrix5(2,3) = Number(1); //c
    loc5->setFlow(flowMatrix5);


    //LOC6
    matrix_t<Number> flowMatrix6 = matrix_t<Number>::Zero(4,4);

    flowMatrix6(0,3) = Number(1); //g
    flowMatrix6(1,3) = Number(0); //x
    flowMatrix6(2,3) = Number(0); //c
    loc6->setFlow(flowMatrix6);


    //trans12
    Reset<Number> reset12;
    vector_t<Number> constantReset12 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset12 = matrix_t<Number>::Identity(3,3);
    linearReset12(0,0) = Number(0);// g = 0
    reset12.setVector(constantReset12);
    reset12.setMatrix(linearReset12);

    trans12->setAggregation(Aggregation::parallelotopeAgg);
    trans12->setSource(loc1);
    trans12->setTarget(loc2);
    trans12->setReset(reset12);

    loc1->addTransition(move(trans12));


    //trans13
    Condition<Number> guard13;
    matrix_t<Number> guardMat13 = matrix_t<Number>::Zero(1,3);
    vector_t<Number> guardVec13 = vector_t<Number>::Zero(1);
    guardVec13(0) = Number(-5);
    guardMat13(0,2) = Number(-1); //c >= 5
    guard13.setMatrix(guardMat13);
    guard13.setVector(guardVec13);

    Reset<Number> reset13;
    vector_t<Number> constantReset13 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset13 = matrix_t<Number>::Identity(3,3);
    linearReset13(2,2) = Number(0);// c = 0
    reset13.setVector(constantReset13);
    reset13.setMatrix(linearReset13);

    trans13->setAggregation(Aggregation::parallelotopeAgg);
    trans13->setGuard(guard13);
    trans13->setSource(loc1);
    trans13->setTarget(loc3);
    trans13->setReset(reset13);

    loc1->addTransition(move(trans13));


    //trans24
    Condition<Number> guard24;
    matrix_t<Number> guardMat24 = matrix_t<Number>::Zero(1,3);
    vector_t<Number> guardVec24 = vector_t<Number>::Zero(1);
    guardVec24(0) = Number(-5);
    guardMat24(0,2) = Number(-1); //c >= 5
    guard24.setMatrix(guardMat24);
    guard24.setVector(guardVec24);

    Reset<Number> reset24;
    vector_t<Number> constantReset24 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset24 = matrix_t<Number>::Identity(3,3);
    linearReset24(2,2) = Number(0);// c = 0
    reset24.setVector(constantReset24);
    reset24.setMatrix(linearReset24);

    trans24->setAggregation(Aggregation::parallelotopeAgg);
    trans24->setGuard(guard24);
    trans24->setSource(loc2);
    trans24->setTarget(loc4);
    trans24->setReset(reset24);

    loc2->addTransition(move(trans24));


    //trans25
    Condition<Number> guard25;
    matrix_t<Number> guardMat25 = matrix_t<Number>::Zero(1,3);
    vector_t<Number> guardVec25 = vector_t<Number>::Zero(1);
    guardVec25(0) = Number(0);
    guardMat25(0,1) = Number(1); //x <= 0
    guard25.setMatrix(guardMat25);
    guard25.setVector(guardVec25);

    trans25->setGuard(guard25);
    trans25->setAggregation(Aggregation::parallelotopeAgg);
    trans25->setSource(loc2);
    trans25->setTarget(loc5);

    loc2->addTransition(move(trans25));


    //trans34
    Reset<Number> reset34;
    vector_t<Number> constantReset34 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset34 = matrix_t<Number>::Identity(3,3);
    linearReset34(0,0) = Number(0);// g = 0
    reset34.setVector(constantReset34);
    reset34.setMatrix(linearReset34);

    trans34->setAggregation(Aggregation::parallelotopeAgg);
    trans34->setSource(loc3);
    trans34->setTarget(loc4);
    trans34->setReset(reset34);

    loc3->addTransition(move(trans34));


    //trans36
    Condition<Number> guard36;
    matrix_t<Number> guardMat36 = matrix_t<Number>::Zero(1,3);
    vector_t<Number> guardVec36 = vector_t<Number>::Zero(1);
    guardVec36(0) = Number(-10);
    guardMat36(0,1) = Number(-1); //x >= 10
    guard36.setMatrix(guardMat36);
    guard36.setVector(guardVec36);

    trans36->setGuard(guard36);
    trans36->setAggregation(Aggregation::parallelotopeAgg);
    trans36->setSource(loc3);
    trans36->setTarget(loc6);

    loc3->addTransition(move(trans36));


    //trans54
    Condition<Number> guard54;
    matrix_t<Number> guardMat54 = matrix_t<Number>::Zero(1,3);
    vector_t<Number> guardVec54 = vector_t<Number>::Zero(1);
    guardVec54(0) = Number(-5);
    guardMat54(0,2) = Number(-1); //c >= 5
    guard54.setMatrix(guardMat54);
    guard54.setVector(guardVec54);

    Reset<Number> reset54;
    vector_t<Number> constantReset54 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset54 = matrix_t<Number>::Identity(3,3);
    linearReset54(2,2) = Number(0);// c = 0
    reset54.setVector(constantReset54);
    reset54.setMatrix(linearReset54);

    trans54->setAggregation(Aggregation::parallelotopeAgg);
    trans54->setGuard(guard54);
    trans54->setSource(loc5);
    trans54->setTarget(loc4);
    trans54->setReset(reset54);

    loc5->addTransition(move(trans54));


    //trans64
    Reset<Number> reset64;
    vector_t<Number> constantReset64 = vector_t<Number>::Zero(3);
    matrix_t<Number> linearReset64 = matrix_t<Number>::Identity(3,3);
    linearReset64(0,0) = Number(0);// g = 0
    reset64.setVector(constantReset64);
    reset64.setMatrix(linearReset64);

    trans64->setAggregation(Aggregation::parallelotopeAgg);
    trans64->setSource(loc6);
    trans64->setTarget(loc4);
    trans64->setReset(reset64);

    loc6->addTransition(move(trans64));



    // create Box holding the initial set.
    matrix_t<Number> boxMat = matrix_t<Number>::Zero(6,3);
    vector_t<Number> boxVec = vector_t<Number>::Zero(6);
    boxMat(0, 0) = Number(-1);
    boxMat(1, 0) = Number(1);
    boxMat(2, 1) = Number(-1);
    boxMat(3, 1) = Number(1);
    boxMat(4, 2) = Number(-1);
    boxMat(5, 2) = Number(1);

    // create initial state.
    minimalAutomaton.addInitialState(loc1.get(), Condition<Number>(boxMat,boxVec));
    minimalAutomaton.addLocation(move(loc1));
    minimalAutomaton.addLocation(move(loc2));
    minimalAutomaton.addLocation(move(loc3));
    minimalAutomaton.addLocation(move(loc4));
    minimalAutomaton.addLocation(move(loc5));
    minimalAutomaton.addLocation(move(loc6));


    HybridAutomaton<Number> newAutomaton(minimalAutomaton);

    EXPECT_EQ((*(newAutomaton.getLocations()[0]->getTransitions()[0]->getTarget())), (*(minimalAutomaton.getLocations()[0]->getTransitions()[0]->getTarget())));

    hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>> reacher(newAutomaton);
    ReachabilitySettings settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.1);
    settings.timeBound = Number(10); //time bound
    settings.jumpDepth = 5;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    // perform reachability analysis.
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



TEST(HybridAutomaton, Bouncingball) {

    using namespace hypro;

    // typedefs for simplification.
    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    // create the discrete structure of the automaton and the automaton itself.
    unique_ptr<Location<Number>> loc1 = make_unique<Location<Number>>();
    unique_ptr<hypro::Transition<Number>> trans = make_unique<hypro::Transition<Number>>();
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
    loc1->addTransition(move(trans));


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
    bBallAutomaton.addInitialState(loc1.get(), Condition<Number>(boxMat,boxVec));
    bBallAutomaton.addLocation(move(loc1));


    // instanciate reachability analysis algorithm.
    hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>> reacher(bBallAutomaton);
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

    std::pair<hypro::HybridAutomaton<Number>, hypro::ReachabilitySettings> ha = std::move(hypro::parseFlowstarFile<Number>("../../test/testfiles/example.model"));


    hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>> reacher(ha.first, ha.second);


    ReachabilitySettings settings = reacher.settings();
    settings.timeBound = Number(10); //time bound
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
/*
TEST(HybridAutomaton, RectangularAutomaton) {
using namespace hypro;

// typedefs for simplification.
typedef mpq_class Number;
typedef HPolytope<Number> Representation;

// create the discrete structure of the automaton and the automaton itself.
unique_ptr<Location<Number>> loc1 = make_unique<Location<Number>>();
HybridAutomaton<Number> rectangularAutomaton = HybridAutomaton<Number>();


// location parameter setting.

// creation of the invariant.
matrix_t<Number> invariantMat = matrix_t<Number>(1,2);
vector_t<Number> invariantVec = vector_t<Number>(1);
invariantVec(0) = Number(0);
invariantMat(0,0) = Number(-1);
invariantMat(0,1) = Number(0);
Condition<Number> inv(invariantMat,invariantVec);
loc1->setInvariant(inv);

// setup rectangular flow
auto& varpool = VariablePool::getInstance();
rectangularFlow<Number> rectFlow;
rectFlow.setFlowIntervalForDimension(carl::Interval<Number>{1,1},varpool.newCarlVariable("t"));
rectFlow.setFlowIntervalForDimension(carl::Interval<Number>{1,3},varpool.newCarlVariable("x"));
loc1->setRectangularFlow(rectFlow);



//// setup flow matrix (3x3, we add an artificial dimension to cope with constants).
//// matrix defining the flow (note: 3rd dimension for constant parts).
//matrix_t<Number> flowMatrix = matrix_t<Number>(3,3);
//flowMatrix(0,0) = Number(0);
//flowMatrix(0,1) = Number(1);
//flowMatrix(0,2) = Number(0);
//flowMatrix(1,0) = Number(0);
//flowMatrix(1,1) = Number(0);
//flowMatrix(1,2) = Number(carl::rationalize<Number>(-9.81));
//flowMatrix(2,0) = Number(0);
//flowMatrix(2,1) = Number(0);
//flowMatrix(2,2) = Number(0);
//loc1->setFlow(flowMatrix);



// setup of the transition.


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
rectangularAutomaton.addInitialState(loc1.get(), Condition<Number>(boxMat,boxVec));
rectangularAutomaton.addLocation(move(loc1));


// instanciate reachability analysis algorithm.
hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>> reacher(rectangularAutomaton);
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
 */
/*
TEST(HybridAutomaton,RectangularAutomatonWorker) {
    // typedefs
    using Number = double;
    using State = hypro::State_t<Number>;

    // settings provider instance as reference for readability
    hypro::SettingsProvider<State>& settingsProvider = hypro::SettingsProvider<State>::getInstance();

    // variables
    auto& varpool = VariablePool::getInstance();
    carl::Variable x = varpool.newCarlVariable("x");
    carl::Variable y = varpool.newCarlVariable("y");
    // rectangular dynamics
    std::map<carl::Variable, carl::Interval<Number>> dynamics;
    dynamics.emplace(std::make_pair(x,carl::Interval<Number>(1,2)));
    dynamics.emplace(std::make_pair(y,carl::Interval<Number>(2,3)));

    // hybrid automaton instance
    hypro::HybridAutomaton<Number> ha;

    // create one location with rectangular flow
    hypro::Location<Number> loc1;
    hypro::rectangularFlow<Number> flow(dynamics);
    loc1.setRectangularFlow(flow);

    // add location
    ha.addLocation(loc1);

    // initial set is a unit box
    hypro::matrix_t<Number> constraints = hypro::matrix_t<Number>(4,2);
    constraints << 1,0,-1,0,0,1,0,-1;
    hypro::vector_t<Number> constants = hypro::vector_t<Number>(4);
    constants << 1,1,1,1;

    ha.addInitialState(&loc1, Condition<Number>(constraints,constants));

    // theoretically we do not need this - check if really needed.
    //settingsProvider.addStrategyElement<hypro::CarlPolytope<Number>>(mpq_class(1)/mpq_class(100), hypro::AGG_SETTING::AGG, -1);
    settingsProvider.addStrategyElement<hypro::CarlPolytope<Number>>(mpq_class(1)/mpq_class(10), hypro::AGG_SETTING::AGG, -1);

    // set settings
    settingsProvider.setHybridAutomaton(std::move(ha));
    hypro::ReachabilitySettings settings;
    settings.useInvariantTimingInformation = false;
    settings.useGuardTimingInformation = false;
    settings.useBadStateTimingInformation = false;
    // settings.timeStep = carl::convert<double,Number>(1.00); does not work
    settings.timeBound = Number(3); //time bound
    settings.jumpDepth = 3;
    settingsProvider.setReachabilitySettings(settings);

    // set up queues
    hypro::WorkQueueManager<std::shared_ptr<hypro::Task<State>>> queueManager;
    auto globalCEXQueue = queueManager.addQueue();
    auto globalQueue = queueManager.addQueue();

    // set up initial states to the correct representation - in this case CarlPolytope
    auto initialStates = settingsProvider.getHybridAutomaton().getInitialStates();
    std::vector<hypro::State_t<Number>> initialStateData;
    for (auto stateMapIt = initialStates.begin(); stateMapIt != initialStates.end(); ++stateMapIt) {

        hypro::State_t<Number> copyState;
        copyState.setLocation(stateMapIt->first);
        copyState.setTimestamp(carl::Interval<hypro::tNumber>(0));

        // if the decider is in use - convert subspaces according to mapping
        State::repVariant _temp;
        _temp = hypro::CarlPolytope<Number>(stateMapIt->second.getMatrix(), stateMapIt->second.getVector());

        copyState.setSet(_temp,0);
        initialStateData.emplace_back(copyState);
    }

    // set up tree
    hypro::ReachTree<State> tree = hypro::ReachTree<State>(new hypro::ReachTreeNode<State>());
    std::vector<hypro::ReachTreeNode<State>*> initialNodes;
    for(const auto& state : initialStateData) {
        hypro::ReachTreeNode<State>* n = new hypro::ReachTreeNode<State>(state,0,tree.getRoot());
        n->setTimestamp(0, carl::Interval<hypro::tNumber>(0));
        initialNodes.push_back(n);
        tree.getRoot()->addChild(n);
    }

    // fill task queue with initial states
    for (const auto& initialNode : initialNodes) {
        globalQueue->enqueue(std::shared_ptr<hypro::Task<State>>(new hypro::Task<State>(initialNode)));
    }

    // create worker
    hypro::ContextBasedReachabilityWorker<State> worker = hypro::ContextBasedReachabilityWorker<State>(settings);

    // data structure to store computed flowpipes
    hypro::Flowpipe<State> segments;

    while(queueManager.hasWorkable(true)) { // locking access to queues.
        auto task = queueManager.getNextWorkable(true,true); // get next task locked and dequeue from front.
        assert(task!=nullptr);
        worker.processTask(task,settingsProvider.getStrategy(), globalQueue, globalCEXQueue, segments);
    }

    // plot flowpipes.
    Plotter<Number>& plotter = Plotter<Number>::getInstance();
    plotter.setFilename("rectangularAutomaton");




//    //for(auto& indexPair : flowpipe){
//        auto flowpipe = segments.getPlotData();
//        auto& set = flowpipe.sets();
//// Plot single flowpipe
//        for(auto& set : flowpipe) {
//            std::vector<Point<Number>> points = set.vertices();
//            if(!points.empty() && points.size() > 2) {
//                for(auto& point : points) {
//                    point.reduceDimension(2);
//                }
//                plotter.addObject(points);
//                points.clear();
//            }
//        }
//    //}
// write output.

    std::vector<hypro::PlotData<State>> plotdata = segments.getPlotData();
    //plotter.addObject(plotdata);
    plotter.plotTex();

}
 */

