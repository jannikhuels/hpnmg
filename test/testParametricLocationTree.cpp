#include "gtest/gtest.h"
#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include "datastructures/Event.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"
#include "PLTWriter.h"

using namespace hpnmg;
using namespace std;

#define MAX_TIME 10

class ParametricLocationEmptyTreeTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ParametricLocationEmptyTreeTest() : parametricLocationTree(ParametricLocation(1,1,1, Event(1)),MAX_TIME) {

    }

    virtual void SetUp() {

    }

    virtual void TearDown() {

    }
};

TEST_F(ParametricLocationEmptyTreeTest, RootNodeTest)
{
    ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
    ASSERT_EQ(rootNode.getNodeID(), ROOT_NODE_ID);
    ASSERT_EQ(rootNode.getParametricLocation().getSourceEvent().getEventType(), EventType::Root);
    ASSERT_EQ(rootNode.getParametricLocation().getSourceEvent().getGeneralDependencies().size(), 1);
}

TEST_F(ParametricLocationEmptyTreeTest, NoChildNodes)
{
    ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
    vector<ParametricLocationTree::Node> childNodes = parametricLocationTree.getChildNodes(rootNode);
    ASSERT_EQ(childNodes.size(), 0);
}

TEST(ParametricLocationTreeTest, InvalidParentNode)
{
    ReadHybridPetrinet reader;
    //TODO Check impl. of reader. Create singletons!
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree = parser.parseHybridPetrinet(hybridPetrinet, 20);
    parametricLocationTree->updateRegions();

    try {
        ParametricLocation location(1,1,1);
        ParametricLocationTree::Node invalidNode(-1,location);
        parametricLocationTree->getChildNodes(invalidNode);
        FAIL() << "This Location is not initialized and an exception should be thrown.";
    } catch(InvalidParentNodeException e) {
        SUCCEED();
    }
}

TEST(ParametricLocationTreeTest, ChildNode)
{
    ReadHybridPetrinet reader;
    //TODO Check impl. of reader. Create singletons!
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree = parser.parseHybridPetrinet(hybridPetrinet, 20);
    parametricLocationTree->updateRegions();

    ParametricLocationTree::Node rootNode = parametricLocationTree->getRootNode();
    vector<ParametricLocationTree::Node> rootChildNodes = parametricLocationTree->getChildNodes(rootNode);
    vector<ParametricLocationTree::Node> stocChildNodes;
    vector<ParametricLocationTree::Node> detChildNodes;
    for (ParametricLocationTree::Node node: rootChildNodes) {
        if (node.getParametricLocation().getSourceEvent().getEventType() == EventType::General) {
            stocChildNodes = parametricLocationTree->getChildNodes(node);
        } else {
            detChildNodes = parametricLocationTree->getChildNodes(node);
        }
    }

    ASSERT_EQ(rootChildNodes.size(), 2);
    ASSERT_EQ(stocChildNodes.size(), 2);
    ASSERT_EQ(detChildNodes.size(), 2);
}

TEST(ParametricLocationTreeTest, RegionTest)
{
    ReadHybridPetrinet reader;
    //TODO Check impl. of reader. Create singletons!
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree = parser.parseHybridPetrinet(hybridPetrinet, 20);
    parametricLocationTree->updateRegions();

    ParametricLocationTree::Node rootNode = parametricLocationTree->getRootNode();
    ASSERT_EQ(2, rootNode.getRegion().dimension());

    //parametricLocationTree.print(true);

    vector_t<double> point = vector_t<double>::Zero(2);
    point << 2,1;
    EXPECT_TRUE(rootNode.getRegion().contains(Point<double>(point)));
    point << 10,0;
    EXPECT_TRUE(rootNode.getRegion().contains(Point<double>(point)));
    point << 1,2;
    EXPECT_FALSE(rootNode.getRegion().contains(Point<double>(point)));

    vector<ParametricLocationTree::Node> rootChildNodes = parametricLocationTree->getChildNodes(rootNode);
    vector<ParametricLocationTree::Node> stocChildNodes;
    for (ParametricLocationTree::Node node: rootChildNodes) {
        if (node.getParametricLocation().getSourceEvent().getEventType() == EventType::General) {
            point << 2,2;
            EXPECT_TRUE(node.getRegion().contains(Point<double>(point)));
            point << 1,2;
            EXPECT_TRUE(node.getRegion().contains(Point<double>(point)));
            point << 1,3;
            EXPECT_FALSE(node.getRegion().contains(Point<double>(point)));
            point << 1,0.5;
            EXPECT_FALSE(node.getRegion().contains(Point<double>(point)));
        } else {
            point << 8,6;
            EXPECT_TRUE(node.getRegion().contains(Point<double>(point)));
            point << 8,4;
            EXPECT_FALSE(node.getRegion().contains(Point<double>(point)));
        }
    }
}

TEST_F(ParametricLocationEmptyTreeTest, IncorrectChildNode)
{
    try {
        ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
        ParametricLocation childNode = rootNode.getParametricLocation();

        Event event;
        event.setEventType(EventType::Immediate);
        childNode.setSourceEvent(event);

        ParametricLocation location(1,1,1);
        ParametricLocationTree::Node invalidNode(-1,location);

        parametricLocationTree.setChildNode(invalidNode, childNode);
        FAIL() << "This is an Invalid Parent Node and thus no Child nodes can be added.";
    } catch(InvalidParentNodeException e) {
        SUCCEED();
    }

}

TEST(ParametricLocationTreeTest, CandidateRegionsForTimeTest) {
    ReadHybridPetrinet reader;
    //TODO Check impl. of reader. Create singletons!
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree = parser.parseHybridPetrinet(hybridPetrinet, 20);
    parametricLocationTree->updateRegions();

    ASSERT_EQ(parametricLocationTree->getCandidateLocationsForTime(3).size(), 3);
    ASSERT_EQ(parametricLocationTree->getCandidateLocationsForTime(6).size(), 4);
    ASSERT_EQ(parametricLocationTree->getCandidateLocationsForTime(9).size(), 5);
    //ASSERT_EQ(parametricLocationTree->getCandidateLocationsForTime(21).size(), 0);
}

TEST(ParametricLocationTreeXML, CreateRegions2D) {
    shared_ptr<hpnmg::ParametricLocationTree> plt = ParseHybridPetrinet{}.parseHybridPetrinet(
        ReadHybridPetrinet{}.readHybridPetrinet("norep_1_1.xml"),
        20
    );
    plt->updateRegions();

    // Each sample pair consists of a test point and the NODE_ID of the PLT node containing the point.
    const vector<pair<Point<double>, NODE_ID>> samples{
        {Point<double>{ 2,  1},  1},
        {Point<double>{ 2,  3},  2},
        {Point<double>{14, 13},  3},
        {Point<double>{ 7, 13},  4},
        {Point<double>{ 1,  3},  5},
        {Point<double>{13, 14},  6},
        {Point<double>{19, 18},  7},
        {Point<double>{ 7, 18},  8},
        {Point<double>{ 1, 13},  9},
        {Point<double>{18, 19}, 10},
    };

    deque<ParametricLocationTree::Node> working_set{plt->getRootNode()};
    while (!working_set.empty()) {
        const ParametricLocationTree::Node node = working_set.front();
        working_set.pop_front();
        const vector<ParametricLocationTree::Node> children = plt->getChildNodes(node);
        working_set.insert(working_set.end(), children.begin(), children.end());

        const STDPolytope<double> &polytope = node.getRegion();
        ASSERT_EQ(polytope.dimension(), 2);

        for (const auto &sample : samples) {
            const auto point = sample.first;
            if (node.getNodeID() == sample.second)
                EXPECT_TRUE(polytope.contains(point)) << "NodeID: " << node.getNodeID() << endl << "polytope: " << polytope << endl << "point: " << point;
            else
                EXPECT_FALSE(polytope.contains(point)) << "NodeID: " << node.getNodeID() << endl << "polytope: " << polytope << endl << "point: " << point;
        }
    }
}

TEST(ParametricLocationTreeXML, CreateRegions3D) {
    shared_ptr<hpnmg::ParametricLocationTree> plt = ParseHybridPetrinet{}.parseHybridPetrinet(
        ReadHybridPetrinet{}.readHybridPetrinet("norep_1_2.xml"),
        20
    );
    plt->updateRegions();

    PLTWriter{}.writePLT(plt, 20);

    // Each sample pair consists of a test point and the NODE_ID of the PLT node containing the point.
    // Each point is the center of its region. This was the simplest way to get 17 example points generated by Mathematica.
    const vector<pair<Point<double>, NODE_ID>> samples{
        {Point<double>{  88/7.,       10.,     36/7.},  1},
        {Point<double>{  17/4.,     47/4.,     31/4.},  2},
        {Point<double>{ 154/9.,       10.,    128/9.},  3},
        {Point<double>{   9/2.,        3.,        9.},  4},
        {Point<double>{  36/5.,   212/15.,   232/15.},  5},
        {Point<double>{677/48.,   727/64.,  1615/96.},  6},
        {Point<double>{ 172/9.,       10.,    164/9.},  7},
        {Point<double>{261/38.,    65/19.,   294/19.},  8},
        {Point<double>{   3/2.,        3.,        9.},  9},
        {Point<double>{107/17.,   144/17.,   294/17.}, 10},
        {Point<double>{703/51.,    35/17.,   914/51.}, 11},
        {Point<double>{169/10., 1331/120.,   229/12.}, 12},
        {Point<double>{784/43., 1346/129., 2464/129.}, 13},
        {Point<double>{ 63/16.,     43/8.,     69/4.}, 14},
        {Point<double>{     2.,        4.,       16.}, 15},
        {Point<double>{     1.,       12.,       18.}, 16},
        {Point<double>{    17.,      5/3.,     58/3.}, 17},
        {Point<double>{    18.,      2/3.,     58/3.}, 18},
    };

    deque<ParametricLocationTree::Node> working_set{plt->getRootNode()};
    while (!working_set.empty()) {
        const ParametricLocationTree::Node node = working_set.front();
        working_set.pop_front();
        const vector<ParametricLocationTree::Node> children = plt->getChildNodes(node);
        working_set.insert(working_set.end(), children.begin(), children.end());

        const STDPolytope<double> &polytope = node.getRegion();
        ASSERT_EQ(polytope.dimension(), 3);

        for (const auto &sample : samples) {
            const auto point = sample.first;
            if (node.getNodeID() == sample.second)
                EXPECT_TRUE(polytope.contains(point)) << "NodeID: " << node.getNodeID() << endl << "polytope: " << polytope << endl << "point: " << point;
            else
                EXPECT_FALSE(polytope.contains(point)) << "NodeID: " << node.getNodeID() << endl << "polytope: " << polytope << endl << "point: " << point;
        }
    }
}

TEST(ParametricLocationTreeXML, CollectCandidatesWithPLT) {
    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, 20);
    plt->updateRegions();
    plt->print(false);

    vector<ParametricLocationTree::Node> candidates;

    candidates = plt->getCandidateLocationsForTime(3);
    ASSERT_EQ(candidates.size(),3);
    ASSERT_EQ(candidates[0].getNodeID(),1);
    ASSERT_EQ(candidates[1].getNodeID(),2);
    ASSERT_EQ(candidates[2].getNodeID(),5);

    candidates = plt->getCandidateLocationsForTime(7.5);
    ASSERT_EQ(candidates.size(),6);
    ASSERT_EQ(candidates[0].getNodeID(),4);
    ASSERT_EQ(candidates[1].getNodeID(),8);
    ASSERT_EQ(candidates[2].getNodeID(),3);
    ASSERT_EQ(candidates[3].getNodeID(),6);
    ASSERT_EQ(candidates[4].getNodeID(),7);
    ASSERT_EQ(candidates[5].getNodeID(),9);

    candidates = plt->getCandidateLocationsForTimeInterval({3.0,7.5});
    ASSERT_EQ(candidates.size(),9);

    candidates = plt->getCandidateLocationsForTimeInterval({6.0,7.0});
    ASSERT_EQ(candidates.size(),4);
    ASSERT_EQ(candidates[0].getNodeID(),4);
    ASSERT_EQ(candidates[1].getNodeID(),8);
    ASSERT_EQ(candidates[2].getNodeID(),3);
    ASSERT_EQ(candidates[3].getNodeID(),6);

    ASSERT_EQ(candidates[0].getParametricLocation().getAccumulatedProbability(),1);

    candidates = plt->getCandidateLocationsForTime(0);
    EXPECT_EQ(3, candidates.size());


}

TEST(ParametricLocationTreeXML, AccumulatedProbability) {
    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("exampleImmediate.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, 18);
    plt->print(false);
    PLTWriter writer;
    writer.writePLT(plt,20);

    vector<ParametricLocationTree::Node> candidates;
    candidates = plt->getCandidateLocationsForTime(1);
    ASSERT_EQ(4, candidates.size());
    ASSERT_EQ(4, candidates[0].getNodeID());
    ASSERT_EQ(5, candidates[1].getNodeID());
    ASSERT_EQ(6, candidates[2].getNodeID());
    ASSERT_EQ(7, candidates[3].getNodeID());

    ASSERT_NEAR(0.11, candidates[0].getParametricLocation().getAccumulatedProbability(),0.01);
    ASSERT_NEAR(0.22, candidates[1].getParametricLocation().getAccumulatedProbability(),0.01);
    ASSERT_NEAR(0.22, candidates[2].getParametricLocation().getAccumulatedProbability(),0.01);
    ASSERT_NEAR(0.44, candidates[3].getParametricLocation().getAccumulatedProbability(),0.01);

    candidates = plt->getCandidateLocationsForTime(0.0);
    EXPECT_EQ(7, candidates.size());
    ASSERT_EQ(1, candidates[0].getNodeID());
    ASSERT_EQ(2, candidates[1].getNodeID());
    ASSERT_EQ(4, candidates[2].getNodeID());
    ASSERT_EQ(5, candidates[3].getNodeID());
    ASSERT_EQ(3, candidates[4].getNodeID());
    ASSERT_EQ(6, candidates[5].getNodeID());
    ASSERT_EQ(7, candidates[6].getNodeID());

    ASSERT_EQ(candidates[0].getParametricLocation().getAccumulatedProbability(),1);
    ASSERT_NEAR(candidates[1].getParametricLocation().getAccumulatedProbability(),0.33,0.01);
    ASSERT_NEAR(candidates[2].getParametricLocation().getAccumulatedProbability(),0.11,0.01);
    ASSERT_NEAR(candidates[3].getParametricLocation().getAccumulatedProbability(),0.22,0.01);
    ASSERT_NEAR(candidates[4].getParametricLocation().getAccumulatedProbability(),0.66,0.01);
    ASSERT_NEAR(candidates[5].getParametricLocation().getAccumulatedProbability(),0.22,0.01);
    ASSERT_NEAR(candidates[6].getParametricLocation().getAccumulatedProbability(),0.44,0.01);

}

TEST(ParametricLocationTreeXML, Normed) {
    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, 10);
    int dim = plt->getDimension();
    ASSERT_EQ(dim, 2);

    ParametricLocationTree::Node r = plt->getRootNode();

    vector<ParametricLocationTree::Node> candidates;
    candidates = plt->getCandidateLocationsForTime(3);


    // GeneralDependencies
    ASSERT_EQ(candidates.size(), 3);
    ASSERT_EQ(candidates[0].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed().size(), 2);
    ASSERT_EQ(candidates[0].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[0], 0);
    ASSERT_EQ(candidates[0].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[1], 0);
    ASSERT_EQ(candidates[1].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed().size(), 2);
    ASSERT_EQ(candidates[1].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[0], 0);
    ASSERT_EQ(candidates[1].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[1], 1);
    ASSERT_EQ(candidates[2].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed().size(), 2);
    ASSERT_EQ(candidates[2].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[0], 0);
    ASSERT_EQ(candidates[2].getParametricLocation().getSourceEvent().getGeneralDependenciesNormed()[1], 2);

//    // Integration Intervals
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals().size(), 1);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].first, 0);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.first.size(), 2);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.first[0], 3);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.first[1], 0);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.second.size(), 2);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.second[0], 10);
//    ASSERT_EQ(candidates[0].getParametricLocation().getIntegrationIntervals()[0].second.second[1], 0);
//
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals().size(), 1);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].first, 0);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.first.size(), 2);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.first[0], 1.5);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.first[1], 0);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.second.size(), 2);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.second[0], 3);
//    ASSERT_EQ(candidates[1].getParametricLocation().getIntegrationIntervals()[0].second.second[1], 0);
}

