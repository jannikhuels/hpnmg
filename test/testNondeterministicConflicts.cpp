#include <iostream>
#include <ctime>
#include "gtest/gtest.h"
#include <chrono>
#include <cstdint>
#include <ctime>


#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include "datastructures/Event.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"
#include "PLTWriter.h"
#include "NondeterminismSolver.h"


#include "../src/hpnmg/optimization/stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../src/hpnmg/optimization/optimization.h"

using namespace hpnmg;
using namespace std;

using namespace alglib;



//Nonprophetic scheduling for Version 1a
TEST(NondeterministicConflicts, Nonprophetic1a) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1a.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 1);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 1b
TEST(NondeterministicConflicts, Nonprophetic1b) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1b.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 1);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 1c
TEST(NondeterministicConflicts, Nonprophetic1c) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1c.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 1);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 1d
TEST(NondeterministicConflicts, Nonprophetic1d) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1d.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 1);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}



//
////Prophetic scheduling for Version 1a
//TEST(NondeterministicConflicts, Prophetic1a) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1a.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 1);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Prophetic scheduling for Version 1b
//TEST(NondeterministicConflicts, Prophetic1b) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1b.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 1);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Prophetic scheduling for Version 1c
//TEST(NondeterministicConflicts, Prophetic1c) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1c.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 1);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Prophetic scheduling for Version 1d
//TEST(NondeterministicConflicts, Prophetic1d) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver1d.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 1);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//


//
////Nonprophetic scheduling for Version 2a
//TEST(NondeterministicConflicts, Nonprophetic2a) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2a.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 2);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 2b
//TEST(NondeterministicConflicts, Nonprophetic2b) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2b.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 2);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 2c
//TEST(NondeterministicConflicts, Nonprophetic2c) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2c.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 2);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 2d
//TEST(NondeterministicConflicts, Nonprophetic2d) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2d.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 2);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//


//Prophetic scheduling for Version 2a
TEST(NondeterministicConflicts, Prophetic2a) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2a.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 2);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}

//Prophetic scheduling for Version 2b
TEST(NondeterministicConflicts, Prophetic2b) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2b.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 2);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Prophetic scheduling for Version 2c
TEST(NondeterministicConflicts, Prophetic2c) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2c.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 2);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Prophetic scheduling for Version 2d
TEST(NondeterministicConflicts, Prophetic2d) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver2d.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 2);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}




//Nonprophetic scheduling for Version 3a
TEST(NondeterministicConflicts, Nonprophetic3a) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;



    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3a.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 3b
TEST(NondeterministicConflicts, Nonprophetic3b) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;



    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3b.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 3c
TEST(NondeterministicConflicts, Nonprophetic3c) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;



    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3c.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Nonprophetic scheduling for Version 3d
TEST(NondeterministicConflicts, Nonprophetic3d) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;


    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3d.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}




//Prophetic scheduling for Version 3a
TEST(NondeterministicConflicts, Prophetic3a) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3a.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Prophetic scheduling for Version 3b
TEST(NondeterministicConflicts, Prophetic3b) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3b.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Prophetic scheduling for Version 3c
TEST(NondeterministicConflicts, Prophetic3c) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3c.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//Prophetic scheduling for Version 3d
TEST(NondeterministicConflicts, Prophetic3d) {

    cout << endl << "Nondeterministic computation started." << endl;
    double maxTime = 24.0;

    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver3d.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
    auto writer = new PLTWriter();
    writer->writePLT(plt, maxTime);

    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
    NondeterminismSolver solver;
    double error;

    clock_t begin0 = clock();

    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 50000, 128, false, true, error, 3);

    clock_t end0 = clock();
    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " seconds" << endl;

    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;

}


//
////Nonprophetic scheduling for Version 4a
//TEST(NondeterministicConflicts, Nonprophetic4a) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver4a.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 4);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 4b
//TEST(NondeterministicConflicts, Nonprophetic4b) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver4b.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 4);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 4c
//TEST(NondeterministicConflicts, Nonprophetic4c) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver4c.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 4);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}
//
//
////Nonprophetic scheduling for Version 4d
//TEST(NondeterministicConflicts, Nonprophetic4d) {
//
//    cout << endl << "Nondeterministic computation started." << endl;
//    double maxTime = 24.0;
//
//
//
//    ReadHybridPetrinet reader;
//    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("../../test/testfiles/nondeterministicModelsCharging/charging_ver4d.xml");
//    ParseHybridPetrinet parser;
//    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, maxTime, 1);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt, maxTime);
//
//    std::vector<ParametricLocationTree::Node> locations = plt->getAllLocations();
//    NondeterminismSolver solver;
//    double error;
//
//    clock_t begin0 = clock();
//
//    double maxprob = solver.solveNondeterminism(plt, plt->getRootNode(), locations, 3, 1000000, 128, false, false, error, 4);
//
//    clock_t end0 = clock();
//    double elapsed_secs = double(end0 - begin0) / CLOCKS_PER_SEC;
//    cout << elapsed_secs << " seconds" << endl;
//
//    cout << "Max probability to take right decision: " << maxprob << " +- " << error << endl;
//
//}



