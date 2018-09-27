#include "STDiagram.h"
#include "ReadHybridPetrinet.h"
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>

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

    clock_t begin = clock();
    auto hybridPetrinet = reader->readHybridPetrinet("../test/testfiles/caseStudy/norep_1_0.xml");
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt, plt->getRootNode()) + 1 << " locations" << endl;
    writer->writePLT(plt, 20);

}