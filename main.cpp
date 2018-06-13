#include "STDiagram.h"
#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>

using namespace hpnmg;
using namespace std;

unsigned long getNodes(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node node) {
    vector<ParametricLocationTree::Node> children = plt->getChildNodes(node);
    unsigned long nodes = children.size();
    for (ParametricLocationTree::Node child : children)
        nodes += getNodes(plt, child);
    return nodes;
}

int main (int argc, char *argv[])
{
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto writer = new PLTWriter();

    cout << "Example 0 without repair and N=0 and t=20:" << endl;
    clock_t begin0 = clock();
    auto hybridPetrinet0 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_0.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 20);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt0, plt0->getRootNode()) + 1 << " locations" << endl;

    cout << "Example 0 without repair and N=1 and t=20:" << endl;
    clock_t begin1 = clock();
    auto hybridPetrinet1 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_1.xml");
    auto plt1 = parser->parseHybridPetrinet(hybridPetrinet1, 20);
    clock_t end1 = clock();
    elapsed_secs = double(end1 - begin1) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt1, plt1->getRootNode()) + 1 << " locations" << endl;

    cout << "Example 0 without repair and N=2 and t=20:" << endl;
    clock_t begin2 = clock();
    auto hybridPetrinet2 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_2.xml");
    auto plt2 = parser->parseHybridPetrinet(hybridPetrinet2, 20);
    clock_t end2 = clock();
    elapsed_secs = double(end2 - begin2) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt2, plt2->getRootNode()) + 1 << " locations" << endl;

    cout << "Example 0 without repair and N=3 and t=20:" << endl;
    clock_t begin3 = clock();
    auto hybridPetrinet3 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_3.xml");
    auto plt3 = parser->parseHybridPetrinet(hybridPetrinet3, 20);
    clock_t end3 = clock();
    elapsed_secs = double(end3 - begin3) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt3, plt3->getRootNode()) + 1 << " locations" << endl;

}