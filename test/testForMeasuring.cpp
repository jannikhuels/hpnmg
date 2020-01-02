
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

TEST(PropertyBasedPLTBuilder, BuildMultipleRegionsDifferentTimes){
    auto	reader	     = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("exampleMultipleRegions.xml");
    auto	builder	     = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();

    //time=0
    const auto	startChecker = std::chrono::high_resolution_clock::now();
     auto	plt	     = builder->parseHybridPetrinet(hybridPetrinet, 10, 0);
    const auto	endChecker   = std::chrono::high_resolution_clock::now();
    auto	initTime     = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
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
    cout << "[Parsing maxTime 6]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

     //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt      = builder->parseHybridPetrinet(hybridPetrinet, 10, 10);
    const auto endChecker10   = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;
}

TEST(PropertyBasedPLTBuilder, BuildTimeTACAS) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("ev_charging.xml");
    auto builder = new PropertyBasedPLTBuilder();
    auto	writer	     = new PLTWriter();

    //time=0
    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 0);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    cout << "[Parsing maxTime 0]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=1
    const auto startChecker1 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 10);
    const auto endChecker1 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker1 - startChecker1).count();
    writer->writePLT(plt, 100, "tacasPLT");
    cout << "[Parsing maxTime 10]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=2
    const auto startChecker2 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 20);
    const auto endChecker2 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker2 - startChecker2).count();
    cout << "[Parsing maxTime 20]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=3
    const auto startChecker3 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 30);
    const auto endChecker3 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker3 - startChecker3).count();
    cout << "[Parsing maxTime 30]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=4
    const auto startChecker4 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 40);
    const auto endChecker4 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker4 - startChecker4).count();
    cout << "[Parsing maxTime 40]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=5
    const auto startChecker5 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 50);
    const auto endChecker5 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker5 - startChecker5).count();
    cout << "[Parsing maxTime 50]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=6
    const auto startChecker6 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 60);
    const auto endChecker6 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker6 - startChecker6).count();
    cout << "[Parsing maxTime 60]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=7
    const auto startChecker7 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 70);
    const auto endChecker7 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker7 - startChecker7).count();
    cout << "[Parsing maxTime 70]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=8
    const auto startChecker8 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 80);
    const auto endChecker8 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker8 - startChecker8).count();
    cout << "[Parsing maxTime 80]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=9
    const auto startChecker9 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 90);
    const auto endChecker9 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker9 - startChecker9).count();
    cout << "[Parsing maxTime 90]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

    //time=10
    const auto startChecker10 = std::chrono::high_resolution_clock::now();
    plt = builder->parseHybridPetrinet(hybridPetrinet, 100, 100);
    const auto endChecker10 = std::chrono::high_resolution_clock::now();
    initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker10 - startChecker10).count();
    cout << "[Parsing maxTime 100]: " << initTime << "ms" << endl;
    cout << "Number of locations: " << plt->numberOfLocations() << endl;

}