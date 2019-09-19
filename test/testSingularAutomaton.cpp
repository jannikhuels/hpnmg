#include <iostream>
#include <ctime>
#include "gtest/gtest.h"
#include <chrono>
#include <cstdint>
#include <ctime>

#include "STDiagram.h"
#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <SingularAutomatonCreator.h>
#include <SingularAutomatonWriter.h>

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





TEST(SingularAutomaton, TestHourglass) {


    ReadHybridPetrinet reader;
    ParseHybridPetrinet parser;
    PLTWriter PLTwriter;
    SingularAutomatonCreator transformer;
    SingularAutomatonWriter automatonWriter;

/**************************
 * The hourglass example. *
 **************************/

// setup
    string filePath = "../../test/testfiles/hourglass.xml";
    double tauMax = 2.5;

// read
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

// transform
    auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, tauMax));
    shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
    shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);

// print
    unsigned long p = getNodes(plt, plt->getRootNode()) + 1;
    unsigned long l = getNumberOfLocations(automaton);
    unsigned long e = getNumberOfEdges(automaton);

    cout << "Hourglass example:" << endl;
    cout << "tau_max=" << tauMax << ", #PL=" << p << ", #Loc=" << l << ", #Edge=" << e << "\n" << endl;

// write
    PLTwriter.writePLT(plt, tauMax, "plt_hourglass");
    automatonWriter.writeAutomaton(automaton, "hourglass");

/**************************************
 * Case study: Water treatment plant. *
 **************************************/

// setup
    vector<int> x = {2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5};
    vector<int> y = {0, 0, 1, 1, 1, 0, 1, 2, 0, 1, 2, 3, 3, 3};
    vector<int> z = {1, 1, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2, 3};
    vector<double> maxTime = {2.5, 3.5, 4, 6.5, 7, 4.5, 5, 5, 5.5, 6, 6, 6, 8, 8.5};

    cout << "Water treatment plant case-study:" << endl;
    cout << " No. | x | y | z |  t  | #PL | #Loc | #Edge" << endl;
    cout << "-----+---+---+---+-----+-----+------+-------" << endl;

    for (int i = 1; i <= x.size(); i++) {
        string name = string("waterTreatmentPlant-") + string(to_string(x[i - 1])) + string("-") +
                      string(to_string(y[i - 1])) + string("-") + string(to_string(z[i - 1]));
        string filePath = string("../../test/testfiles/caseStudy-waterTreatmentPlant/") + name + string(".xml");

        // read
        shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);

        // transform with individual maxTime
        auto treeAndAutomaton(transformer.transformIntoSingularAutomaton(hybridPetriNet, maxTime[i - 1]));
        shared_ptr<ParametricLocationTree> plt(treeAndAutomaton.first);
        shared_ptr<SingularAutomaton> automaton(treeAndAutomaton.second);

        // print
        p = getNodes(plt, plt->getRootNode()) + 1;
        l = getNumberOfLocations(automaton);
        e = getNumberOfEdges(automaton);

        cout << "  " << setw(2) << i << " | " << x[i - 1] << " | " << y[i - 1] << " | " << z[i - 1] << " | "
             << setw(3) << maxTime[i - 1] << " | " << setw(3) << p << " |  " << setw(2) << l << "  |  "
             << setw(2) << e << endl;

        double maxTimeConst = 8.5;

        // transform with constant maxTime
        auto treeAndAutomatonConst(transformer.transformIntoSingularAutomaton(hybridPetriNet, maxTimeConst));
        shared_ptr<ParametricLocationTree> pltConst(treeAndAutomatonConst.first);
        shared_ptr<SingularAutomaton> automatonConst(treeAndAutomatonConst.second);

        // print
        p = getNodes(pltConst, pltConst->getRootNode()) + 1;
        l = getNumberOfLocations(automatonConst);
        e = getNumberOfEdges(automatonConst);

        cout << "     |   |   |   | " << setw(3) << maxTimeConst << " | " << setw(3) << p << " |  " << setw(2) << l
             << "  |  " << setw(2) << e << endl;


        // write
        string filenamePLT = "plt_" + name;
        PLTwriter.writePLT(plt, maxTime[i - 1], filenamePLT);
        automatonWriter.writeAutomaton(automaton, name);

    }

}
