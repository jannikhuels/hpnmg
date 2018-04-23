#include "STDiagram.h"
#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>

int main (int argc, char *argv[])
{
    using namespace hpnmg;
    using namespace std;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto writer = new PLTWriter();

    cout << "Example with 0 general input transitions:" << endl;
    clock_t begin0 = clock();
    auto hybridPetrinet0 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/waterTreatment.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 20);
    writer->writePLT(plt0, 20);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
}