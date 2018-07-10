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

    cout << "Example 0 with repair and N=0 and t=10:" << endl;
    clock_t begin0 = clock();
    auto hybridPetrinet0 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_0.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt0, plt0->getRootNode()) + 1 << " locations" << endl;

    cout << "Example 0 with repair and N=1 and t=10:" << endl;
    clock_t begin1 = clock();
    auto hybridPetrinet1 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/rep_1_1.xml");
    auto plt1 = parser->parseHybridPetrinet(hybridPetrinet1, 10);
    clock_t end1 = clock();
    elapsed_secs = double(end1 - begin1) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
//    writer->writePLT(plt1, 10);
    cout << getNodes(plt1, plt1->getRootNode()) + 1 << " locations" << endl;

//    cout << "Example 1 with repair and N=2 and t=10:" << endl;
//    clock_t begin2 = clock();
//    auto hybridPetrinet2 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/rep_2_2.xml");
//    auto plt2 = parser->parseHybridPetrinet(hybridPetrinet2, 10);
//    clock_t end2 = clock();
//    elapsed_secs = double(end2 - begin2) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
////    writer->writePLT(plt2, 10);
//    cout << getNodes(plt2, plt2->getRootNode()) + 1 << " locations" << endl;
//////
    cout << "Example 1 with repair and N=3 and t=10:" << endl;
    clock_t begin3 = clock();
    auto hybridPetrinet3 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/rep_2_3.xml");
    auto plt3 = parser->parseHybridPetrinet(hybridPetrinet3, 10);
    clock_t end3 = clock();
    elapsed_secs = double(end3 - begin3) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt3, plt3->getRootNode()) + 1 << " locations" << endl;

//    cout << "Example 0 without repair and N=4 and t=10:" << endl;
//    clock_t begin4 = clock();
//    auto hybridPetrinet4 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_4.xml");
//    auto plt4 = parser->parseHybridPetrinet(hybridPetrinet4, 10);
//    clock_t end4 = clock();
//    elapsed_secs = double(end4 - begin4) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt4, plt4->getRootNode()) + 1 << " locations" << endl;
//
//    cout << "Example 0 without repair and N=5 and t=10:" << endl;
//    clock_t begin5 = clock();
//    auto hybridPetrinet5 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_1_5.xml");
//    auto plt5 = parser->parseHybridPetrinet(hybridPetrinet5, 10);
//    clock_t end5 = clock();
//    elapsed_secs = double(end5 - begin5) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt5, plt5->getRootNode()) + 1 << " locations" << endl;
//
//    cout << "Example 2 without repair and N=2 and t=10:" << endl;
//    clock_t begin6 = clock();
//    auto hybridPetrinet6 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_2_2.xml");
//    auto plt6 = parser->parseHybridPetrinet(hybridPetrinet6, 10);
//    clock_t end6 = clock();
//    elapsed_secs = double(end6 - begin6) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt6, plt6->getRootNode()) + 1 << " locations" << endl;
//
//    cout << "Example 2 without repair and N=3 and t=10:" << endl;
//    clock_t begin7 = clock();
//    auto hybridPetrinet7 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_2_3.xml");
//    auto plt7 = parser->parseHybridPetrinet(hybridPetrinet7, 10);
//    clock_t end7 = clock();
//    elapsed_secs = double(end7 - begin7) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt7, plt7->getRootNode()) + 1 << " locations" << endl;
//
//    cout << "Example 2 without repair and N=4 and t=10:" << endl;
//    clock_t begin8 = clock();
//    auto hybridPetrinet8 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_2_4.xml");
//    auto plt8 = parser->parseHybridPetrinet(hybridPetrinet8, 10);
//    clock_t end8 = clock();
//    elapsed_secs = double(end8 - begin8) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt8, plt8->getRootNode()) + 1 << " locations" << endl;
//
//    cout << "Example 2 without repair and N=5 and t=10:" << endl;
//    clock_t begin9 = clock();
//    auto hybridPetrinet9 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/caseStudy/norep_2_5.xml");
//    auto plt9 = parser->parseHybridPetrinet(hybridPetrinet9, 10);
//    clock_t end9 = clock();
//    elapsed_secs = double(end9 - begin9) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//    cout << getNodes(plt9, plt9->getRootNode()) + 1 << " locations" << endl;

}