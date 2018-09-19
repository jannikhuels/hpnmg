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

    clock_t begin0 = clock();
    auto hybridPetrinet0 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/battery_simple_v3.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 20);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
    cout << getNodes(plt0, plt0->getRootNode()) + 1 << " locations" << endl;
    writer->writePLT(plt0, 20);

}