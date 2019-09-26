#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>
#include <ProbabilityCalculator.h>
#include "ParametricLocationTree.h"
#include <SingularAutomatonCreator.h>
#include <SingularAutomatonWriter.h>
#include "gtest/gtest.h"
#include "PLTWriter.h"
#include <chrono>


using namespace hpnmg;
using namespace std;



unsigned long getNodes(const shared_ptr<ParametricLocationTree>& plt, const ParametricLocationTree::Node& node) {
    vector<ParametricLocationTree::Node> children = plt->getChildNodes(node);
    unsigned long nodes = children.size();
    for (const ParametricLocationTree::Node& child : children)
        nodes += getNodes(plt, child);
    return nodes;



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




TEST(HeatedTank, probability){

>>>>>>> hybridAutomaton
   double maxTime = 100.0;

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("../../test/testfiles/heatedTank/heatedtank_v2.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, maxTime);

   auto writer = new PLTWriter();
      writer->writePLT(plt0, maxTime);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(maxTime);

   double error;
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, maxTime, 10000, error);

   ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}



TEST(HeatedTank, automaton) {


    ReadHybridPetrinet reader;
    PLTWriter PLTwriter;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;


// setup
    string filePath = "../../test/testfiles/heatedTank/heatedtank_v2.xml";
    double tauMax = 1000.0;

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

    cout << "Heated Tank:" << endl;
    cout << "tau_max=" << tauMax << ", #PL=" << p << ", #Loc=" << l << ", #Edge=" << e << "\n" << endl;

// write
    PLTwriter.writePLT(plt, tauMax, "plt_heatedtank_v2_100");
    automatonWriter.writeAutomaton(automaton, "heatedtank_v2_100");

}
