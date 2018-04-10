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
    auto hybridPetrinet0 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik0general.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 20);
    writer->writePLT(plt0, 20);
    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Example with 1 general input transitions:" << endl;
    clock_t begin1 = clock();
    auto hybridPetrinet1 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik1general.xml");
    auto plt1 = parser->parseHybridPetrinet(hybridPetrinet1, 20);
    writer->writePLT(plt1, 20);
    clock_t end1 = clock();
    elapsed_secs = double(end1 - begin1) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Example with 2 general input transitions:" << endl;
    clock_t begin2 = clock();
    auto hybridPetrinet2 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik2general.xml");
    auto plt2 = parser->parseHybridPetrinet(hybridPetrinet2, 20);
    writer->writePLT(plt2, 20);
    clock_t end2 = clock();
    elapsed_secs = double(end2 - begin2) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Example with 3 general input transitions:" << endl;
    clock_t begin3 = clock();
    auto hybridPetrinet3 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik3general.xml");
    auto plt3 = parser->parseHybridPetrinet(hybridPetrinet3, 20);
    writer->writePLT(plt3, 20);
    clock_t end3 = clock();
    elapsed_secs = double(end3 - begin3) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Example with 4 general input transitions:" << endl;
    clock_t begin4 = clock();
    auto hybridPetrinet4 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik4general.xml");
    auto plt4 = parser->parseHybridPetrinet(hybridPetrinet4, 20);
    writer->writePLT(plt4, 20);
    clock_t end4 = clock();
    elapsed_secs = double(end4 - begin4) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Example with 5 general input transitions:" << endl;
    clock_t begin5 = clock();
    auto hybridPetrinet5 = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/jannik5general.xml");
    auto plt5 = parser->parseHybridPetrinet(hybridPetrinet5, 20);
    writer->writePLT(plt5, 20);
    clock_t end5 = clock();
    elapsed_secs = double(end5 - begin5) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;
}