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

    parametricLocationTree->updateRegions();
    ParametricLocationTree::Node rootNode = parametricLocationTree->getRootNode();
    ASSERT_EQ(rootNode.getRegion().hPolytope.dimension(), 2);

    //parametricLocationTree.print(true);

    vector_t<double> point = vector_t<double>::Zero(2);
    point << 2,1;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), true);
    point << 10,0;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), true);
    point << 1,2;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), false);

    vector<ParametricLocationTree::Node> rootChildNodes = parametricLocationTree->getChildNodes(rootNode);
    vector<ParametricLocationTree::Node> stocChildNodes;
    for (ParametricLocationTree::Node node: rootChildNodes) {
        if (node.getParametricLocation().getSourceEvent().getEventType() == EventType::General) {
            point << 2,2;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), true);
            point << 1,2;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), true);
            point << 1,3;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), false);
            point << 1,0.5;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), false);
        } else {
            point << 8,6;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), true);
            point << 8,4;
            ASSERT_EQ(node.getRegion().hPolytope.contains(Point<double>(point)), false);
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

TEST(ParametricLocationTreeXML, CreateRegions) {
    ReadHybridPetrinet reader;
    //TODO Check impl. of reader. Create singletons!
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, 20);
    plt->updateRegions();


    plt->print(false);

    ParametricLocationTree::Node rootNode = plt->getRootNode();
    ASSERT_EQ(rootNode.getRegion().hPolytope.dimension(), 2);

    vector_t<double> point = vector_t<double>::Zero(2);
    point << 2,1;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), true);
    point << 20,0;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), true);
    point << 1,2;
    ASSERT_EQ(rootNode.getRegion().hPolytope.contains(Point<double>(point)), false);

    //TODO Extended checks for example.xml
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

