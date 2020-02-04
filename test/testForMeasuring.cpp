
#include <hpnmg/PLTWriter.h>
#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"
#include "PropertyBasedPLTBuilder.h"
#include "ParseHybridPetrinet.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Negation.h"
#include "modelchecker/Until.h"
#include "modelchecker/Conjunction.h"
#include <chrono>
#include <memory>

using namespace hpnmg;

TEST(PropertyBasedPLTBuilder, testLisa2){
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("Lisa2.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto writer = new PLTWriter();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 5, 5);

    writer->writePLT(plt,5,"Lisa2PLT");
}

TEST(PropertyBasedPLTBuilder, BuildPLT) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("annabell_example.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto writer = new PLTWriter();

    const auto startChecker0 = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 4);
    const auto endChecker0 = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker0 - startChecker0).count();
    writer->writePLT(plt,100,"AnnabellExamplePLT");
    cout << "[Vergleich ParseHybridPetrinet]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
}

TEST(PropertyBasedPLTBuilder, BuildMultipleRegionsDifferentTimes){
    auto	reader	     = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("exampleMultipleRegions.xml");
    auto	builder	     = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();
    auto parser = new ParseHybridPetrinet();

    //vergleich mit ParseHybridPetrinet
    const auto	startChecker0 = std::chrono::high_resolution_clock::now();
     auto	plt	     = parser->parseHybridPetrinet(hybridPetrinet, 10);
    const auto	endChecker0   = std::chrono::high_resolution_clock::now();
    auto	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker0 - startChecker0).count();
    cout << "[Vergleich ParseHybridPetrinet]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=0
    const auto	startChecker = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 0);
    const auto	endChecker   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto	startChecker1 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 1);
    const auto	endChecker1  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    cout << "[Parsing maxTime 1]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto	startChecker2 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 2);
    const auto	endChecker2  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
    writer->writePLT(plt, 10, "multipleRegionsPLT");

    //time=3
    const auto	startChecker3 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 3);
    const auto	endChecker3   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 3]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto	startChecker4 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 4);
    const auto	endChecker4   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 4]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 5);
    const auto endChecker5   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 5]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto	startChecker6 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 6);
    const auto	endChecker6   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto	startChecker7 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 7);
    const auto	endChecker7   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 7]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto	startChecker8 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 8);
    const auto	endChecker8   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 8]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto	startChecker9 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 9);
    const auto	endChecker9   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 9]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 10, 10);
    const auto endChecker10   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
}

TEST(PropertyBasedPLTBuilder, MultipleRegionsFormula1){
    auto	reader	     = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("exampleMultipleRegions.xml");
    auto	builder	     = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();
    auto formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("pd3", 0)), 10));

    //time=0
    const auto	startChecker = std::chrono::high_resolution_clock::now();
     auto	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 0, 0, formula);
    const auto	endChecker   = std::chrono::high_resolution_clock::now();
    auto	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto	startChecker1 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 1, 0, formula);
    const auto	endChecker1  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    cout << "[Parsing maxTime 1]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto	startChecker2 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 2, 0, formula);
    const auto	endChecker2  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=3
    const auto	startChecker3 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 3, 0, formula);
    const auto	endChecker3   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 3]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto	startChecker4 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 4, 0, formula);
    const auto	endChecker4   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 4]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 5, 0, formula);
    const auto endChecker5   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 5]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto	startChecker6 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 6, 0, formula);
    const auto	endChecker6   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto	startChecker7 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 7, 0, formula);
    const auto	endChecker7   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 7]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto	startChecker8 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 8, 0, formula);
    const auto	endChecker8   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 8]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto	startChecker9 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 9, 0, formula);
    const auto	endChecker9   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 10, 10, 0, formula);
    const auto endChecker10   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}

TEST(PropertyBasedPLTBuilder, MultipleRegionsFormula2) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("exampleMultipleRegions.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto writer = new PLTWriter();
    auto formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)), 10));

    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 1, 0, formula);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing trueUpd1=0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<True>()), Formula(std::make_shared<Conjunction>(Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 6)),
                    Formula(std::make_shared<DiscreteAtomicProperty>("pd4", 5)))), 10));

    const auto startChecker2 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 1, 0, formula);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing trueU(pd1=6+pd4=5)]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pd3", 1)), Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 6)), 10));
    const auto startChecker3 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 1, 0, formula);
    const auto endChecker3 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing d3=1Ud1=6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 8)), Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 4)), 10));
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 2, 0, formula);
    const auto endChecker5 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing d1=8Ud2=4, T2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 4)), 10));
    const auto startChecker6 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 2, 0, formula);
    const auto endChecker6 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing trueUpd2=4, T2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}
//test for annabell_example
TEST(PropertyBasedPLTBuilder, BuildAnnabellExample){
    auto	reader	     = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("annabell_example.xml");
    auto	builder	     = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();
    auto parser = new ParseHybridPetrinet();

    //vergleich mit ParseHybridPetrinet
    const auto	startChecker0 = std::chrono::high_resolution_clock::now();
     auto	plt	     = parser->parseHybridPetrinet(hybridPetrinet, 10);
    const auto	endChecker0   = std::chrono::high_resolution_clock::now();
    auto	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker0 - startChecker0).count();
    cout << "[Vergleich ParseHybridPetrinet]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=0
    const auto	startChecker = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 0);
    const auto	endChecker   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto	startChecker1 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 1);
    const auto	endChecker1  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    cout << "[Parsing maxTime 1]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto	startChecker2 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 2);
    const auto	endChecker2  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
    writer->writePLT(plt, 10, "multipleRegionsPLT");

    //time=3
    const auto	startChecker3 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 3);
    const auto	endChecker3   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 3]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto	startChecker4 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 4);
    const auto	endChecker4   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 4]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 5);
    const auto endChecker5   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 5]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto	startChecker6 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 6);
    const auto	endChecker6   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto	startChecker7 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 7);
    const auto	endChecker7   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 7]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto	startChecker8 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 8);
    const auto	endChecker8   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 8]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto	startChecker9 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 9);
    const auto	endChecker9   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 9]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 30, 10);
    const auto endChecker10   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    const auto	startChecker11 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 11);
    const auto	endChecker11  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker11 - startChecker11).count();
    cout << "[Parsing maxTime 11]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto	startChecker12 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 12);
    const auto	endChecker12  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker12 - startChecker12).count();
    cout << "[Parsing maxTime 12]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;


    //time=3
    const auto	startChecker13 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 13);
    const auto	endChecker13   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker13 - startChecker13).count();
    cout << "[Parsing maxTime 13]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto	startChecker14 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 14);
    const auto	endChecker14   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker14 - startChecker14).count();
    cout << "[Parsing maxTime 14]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=5
    const auto startChecker15 = std::chrono::high_resolution_clock::now();
    plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 15);
    const auto endChecker15   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker15 - startChecker15).count();
    cout << "[Parsing maxTime 15]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto	startChecker16 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 16);
    const auto	endChecker16   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker16 - startChecker16).count();
    cout << "[Parsing maxTime 16]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto	startChecker17 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 17);
    const auto	endChecker17   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker17 - startChecker17).count();
    cout << "[Parsing maxTime 17]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto	startChecker18 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 18);
    const auto	endChecker18   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker18 - startChecker18).count();
    cout << "[Parsing maxTime 18]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto	startChecker19 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 19);
    const auto	endChecker19   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker19 - startChecker19).count();
    cout << "[Parsing maxTime 19]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker20 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 30, 20);
    const auto endChecker20   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker20 - startChecker20).count();
    cout << "[Parsing maxTime 20]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
    /*
    const auto	startChecker21 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 21);
    const auto	endChecker21  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker21 - startChecker21).count();
    cout << "[Parsing maxTime 21]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto	startChecker22 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 22);
    const auto	endChecker22  = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker22 - startChecker22).count();
    cout << "[Parsing maxTime 22]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;


    //time=3
    const auto	startChecker23 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 23);
    const auto	endChecker23   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker23 - startChecker23).count();
    cout << "[Parsing maxTime 23]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto	startChecker24 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 24);
    const auto	endChecker24   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker24 - startChecker24).count();
    cout << "[Parsing maxTime 24]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=5
    const auto startChecker25 = std::chrono::high_resolution_clock::now();
    plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 25);
    const auto endChecker25   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker25 - startChecker25).count();
    cout << "[Parsing maxTime 25]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto	startChecker26 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 26);
    const auto	endChecker26   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker26 - startChecker26).count();
    cout << "[Parsing maxTime 26]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto	startChecker27 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 27);
    const auto	endChecker27   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker27 - startChecker27).count();
    cout << "[Parsing maxTime 27]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto	startChecker28 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 28);
    const auto	endChecker28   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker28 - startChecker28).count();
    cout << "[Parsing maxTime 28]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto	startChecker29 = std::chrono::high_resolution_clock::now();
     	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 30, 29);
    const auto	endChecker29   = std::chrono::high_resolution_clock::now();
    	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker29 - startChecker29).count();
    cout << "[Parsing maxTime 29]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker30 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 30, 30);
    const auto endChecker30   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker30 - startChecker30).count();
    cout << "[Parsing maxTime 30]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
*/    
}

TEST(PropertyBasedPLTBuilder, testAnnabellExampleFormulas) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("annabell_example.xml");
    auto builder = new PropertyBasedPLTBuilder();
    //auto writer = new PLTWriter();
    auto formula = Formula(std::make_shared<Until>(Formula(std::make_shared<True>()),
            Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 10)))), 30));

    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 20, 1, 0, formula);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing trueU pc1>=10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(Formula(std::make_shared<DiscreteAtomicProperty>("pd1",1)),
            Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 10)))), 30));
    const auto startChecker2 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 20, 2, 0, formula);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing pd1=1 U(pc1>=10), T2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(Formula(std::make_shared<True>())
            , Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 11)), 30));
    const auto startChecker3 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 20, 1, 0, formula);
    const auto endChecker3 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing true U pd1=11]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2))))
            , Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 11)), 30));
    const auto startChecker4 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 20, 3, 0, formula);
    const auto endChecker4 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing pc1>=2 U pd1=11, T3]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(Formula(std::make_shared<True>())
            , Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 2)), 30));
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 20, 1, 0, formula);
    const auto endChecker5 = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing true U pd1=2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}
//begin tests for ev_charging

TEST(PropertyBasedPLTBuilder, BuildTimeTACAS) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("ev_charging.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();
    auto parser = new ParseHybridPetrinet();

    //comparison time ParseHybridPetrinet
    const auto startChecker0 = std::chrono::high_resolution_clock::now();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 10);
    const auto endChecker0 = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker0 - startChecker0).count();
    cout << "[Comparison with ParseHybridPetrinet]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=0
    const auto startChecker = std::chrono::high_resolution_clock::now();
     plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 0);
    const auto endChecker = std::chrono::high_resolution_clock::now();
     initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto startChecker1 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 1);
    const auto endChecker1 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    writer->writePLT(plt, 100, "tacasPLT");
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto startChecker2 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 2);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 20]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=3
    const auto startChecker3 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 3);
    const auto endChecker3 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 30]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto startChecker4 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 4);
    const auto endChecker4 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 40]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 5);
    const auto endChecker5 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 50]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto startChecker6 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 6);
    const auto endChecker6 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 60]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto startChecker7 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 7);
    const auto endChecker7 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 70]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto startChecker8 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 8);
    const auto endChecker8 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 80]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto startChecker9 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 9);
    const auto endChecker9 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 90]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 10, 10);
    const auto endChecker10 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 100]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}

TEST(PropertyBasedPLTBuilder, ev_chargingFormula1) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("ev_charging.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto writer = new PLTWriter();
    auto formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("in_lot", 0)), 100));

    //time=0
    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 0, 0, formula);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto startChecker1 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 1, 0, formula);
    const auto endChecker1 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    cout << "[Parsing maxTime 1]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto startChecker2 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 2, 0, formula);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 2]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
    writer->writePLT(plt, 10, "multipleRegionsPLT");

    //time=3
    const auto startChecker3 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 3, 0, formula);
    const auto endChecker3 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 3]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto startChecker4 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 4, 0, formula);
    const auto endChecker4 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 4]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 5, 0, formula);
    const auto endChecker5 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 5]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto startChecker6 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 6, 0, formula);
    const auto endChecker6 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto startChecker7 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 7, 0, formula);
    const auto endChecker7 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 7]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto startChecker8 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 8, 0, formula);
    const auto endChecker8 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 8]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto startChecker9 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 9, 0, formula);
    const auto endChecker9 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 10, 0, formula);
    const auto endChecker10 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}

TEST(PropertyBasedPLTBuilder, ev_chargingFormulas2){

    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("ev_charging.xml");
    auto builder = new PropertyBasedPLTBuilder();
    //auto writer = new PLTWriter();

    auto formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<True>()), Formula(std::make_shared<Conjunction>(Formula(std::make_shared<DiscreteAtomicProperty>("grid_policy_discharging", 0)),
                    Formula(std::make_shared<DiscreteAtomicProperty>("grid_policy_charging", 0)))) , 100));

    const auto startChecker2 = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 1, 0, formula);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing formula both general transitions fired]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    formula = Formula(std::make_shared<Until>(
            Formula(std::make_shared<Conjunction>(Formula(std::make_shared<DiscreteAtomicProperty>("grid_policy_discharging", 1)),
                    Formula(std::make_shared<DiscreteAtomicProperty>("grid_policy_charging", 1))))
        , Formula(std::make_shared<ContinuousAtomicProperty>("ev", 10)), 100));

    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 10, 0, formula);
    const auto endChecker10 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing formula ev<=10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}
