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
#include <ProbabilityOnReachableSets.h>
#include "gtest/gtest.h"
#include "PLTWriter.h"
#include <chrono>
#include "NondeterminismSolver.h"

#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "datastructures/Point.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"
#include "parser/antlr4-flowstar/ParserWrapper.h"


#include <bits/stdc++.h>
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


TEST(HybridAutomaton, NondeterministicConflict) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/examplesHybridAutomata/exampleNondeterminism4.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 5);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

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

    clock_t begin0 = clock();
    auto plt0 = parser->parseHybridPetrinet(hybridPetriNet, tauMax);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << "PLT build: " << elapsed_secs << " seconds" << endl;

// transform
    begin0 = clock();
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tauMax));
    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    shared_ptr<SingularAutomaton> automaton = transformer.addDistributions(treeAndAutomaton.second, plt0->getDistributionsNormalized());
    //shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
    end0 = clock();
    elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << "Transformation: " << elapsed_secs << " seconds" << endl;

// print
    unsigned long p = getNodes(plt, plt->getRootNode()) + 1;
    unsigned long l = getNumberOfLocations(automaton);
    unsigned long e = getNumberOfEdges(automaton);

    cout << "t_max=" << tauMax << ", #PL=" << p << ", #Loc=" << l << ", #Edge=" << e << "\n" << endl;

// write
    PLTwriter.writePLT(plt, tauMax, "plt_example");

    begin0 = clock();
    automatonWriter.writeAutomaton(automaton, "example");
    end0 = clock();
    elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << "Write JANI: " << elapsed_secs << " seconds" << endl;
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

    std::pair<hypro::HybridAutomaton<Number>, hypro::ReachabilitySettings> ha = std::move(hypro::parseFlowstarFile<Number>("../../test/testfiles/examplesHybridAutomata/examplePauline2.model"));


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
                        for (int i = 0; i < point.rawCoordinates().size(); i++){
                            auto coordinate = point.rawCoordinates()[i];
                            cout << carl::convert<Number, double>(coordinate) << ", ";
                        }
                        cout << endl;
                        point.reduceDimension(2);
                    }
                    cout << "new set" << endl;
                    plotter.addObject(points);
                    points.clear();
                }
            }
        }
        // write output.
        plotter.plotTex();

}


//flow pipe reachability for HPnGs
TEST(HybridAutomaton, converter) {

    using namespace hypro;
    typedef HPolytope<Number> Representation;

    ReadHybridPetrinet reader;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;

// setup
    string filePath = "../../test/testfiles/examplesHybridAutomata/exampleHybrid2.xml";
    double tMax = 10.0;

// read HPnG
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

// transform HPnG into SingularAutomaton
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tMax));

// Get and export PLT (not necessary for reachability)
    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    auto writer = new PLTWriter();
    writer->writePLT(plt, tMax);

// Pass Singular Automaton to Handler
    shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);
    HybridAutomatonHandler handler(automaton, tMax);

// Compute flowpipes
    auto flowpipes = handler.computeFlowpipes(tMax, 0.01, 5);

    handler.plotTex("example", flowpipes);

}