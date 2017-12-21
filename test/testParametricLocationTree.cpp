#include "gtest/gtest.h"
#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include "datastructures/Event.h"

using namespace hpnmg;

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

class ParametricLocationTreeTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ParametricLocationTreeTest() : parametricLocationTree(ParametricLocation(1,1,1, Event(1)), MAX_TIME) {

    }

    virtual void SetUp() {

        ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
        ParametricLocation childNode = rootNode.getParametricLocation();

        ParametricLocationTree::Node detNode = parametricLocationTree.setChildNode(rootNode, ParametricLocation(1,1,1,Event(EventType::Timed, vector<double>{0}, 5)));
        ParametricLocationTree::Node stocNode = parametricLocationTree.setChildNode(rootNode, ParametricLocation(1,1,1,Event(EventType::General, vector<double>{-1}, 0)));
        ParametricLocationTree::Node emptyNode = parametricLocationTree.setChildNode(stocNode, ParametricLocation(1,1,1,Event(EventType::General, vector<double>{-2}, 0)));
        parametricLocationTree.setChildNode(stocNode, ParametricLocation(1,1,1,Event(EventType::Timed, vector<double>{0}, 5), std::vector<double> {0}, std::vector<double> {2.5}));
        parametricLocationTree.setChildNode(emptyNode, ParametricLocation(1,1,1,Event(EventType::Timed, vector<double>{0}, 5), std::vector<double> {2.5}, std::vector<double> {5}));
        parametricLocationTree.setChildNode(detNode, ParametricLocation(1,1,1,Event(EventType::General, vector<double>{-1}, 0), std::vector<double> {5}, std::vector<double> {7.5}));
        ParametricLocationTree::Node fullNode = parametricLocationTree.setChildNode(detNode, ParametricLocation(1,1,1,Event(EventType::Timed, vector<double>{0}, 7.5)));
        parametricLocationTree.setChildNode(fullNode, ParametricLocation(1,1,1,Event(EventType::Timed, vector<double>{-1}, 0), std::vector<double> {7.5}, std::vector<double> {10}));
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
    std::vector<ParametricLocationTree::Node> childNodes = parametricLocationTree.getChildNodes(rootNode);
    ASSERT_EQ(childNodes.size(), 0);
}

TEST_F(ParametricLocationTreeTest, InvalidParentNode)
{
    try {
        ParametricLocation location(1,1,1);
        ParametricLocationTree::Node invalidNode(-1,location);
        parametricLocationTree.getChildNodes(invalidNode);
        FAIL() << "This Location is not initialized and an exception should be thrown.";
    } catch(InvalidParentNodeException e) {
        SUCCEED();
    }
}

TEST_F(ParametricLocationTreeTest, ChildNode)
{
    ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
    std::vector<ParametricLocationTree::Node> rootChildNodes = parametricLocationTree.getChildNodes(rootNode);
    std::vector<ParametricLocationTree::Node> stocChildNodes;
    std::vector<ParametricLocationTree::Node> detChildNodes;
    for (ParametricLocationTree::Node node: rootChildNodes) {
        if (node.getParametricLocation().getSourceEvent().getEventType() == EventType::General) {
            stocChildNodes = parametricLocationTree.getChildNodes(node);
        } else {
            detChildNodes = parametricLocationTree.getChildNodes(node);
        }
    }

    ASSERT_EQ(rootChildNodes.size(), 2);
    ASSERT_EQ(stocChildNodes.size(), 2);
    ASSERT_EQ(detChildNodes.size(), 2);
}

TEST_F(ParametricLocationTreeTest, RegionTest)
{
    parametricLocationTree.updateRegions();
    ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
    ASSERT_EQ(rootNode.getRegion().dimension(), 2);

    //parametricLocationTree.print(true);

    vector_t<double> point = vector_t<double>::Zero(2);
    point << 2,1;
    ASSERT_EQ(rootNode.getRegion().contains(Point<double>(point)), true);
    point << 10,0;
    ASSERT_EQ(rootNode.getRegion().contains(Point<double>(point)), true);
    point << 1,2;
    ASSERT_EQ(rootNode.getRegion().contains(Point<double>(point)), false);

    std::vector<ParametricLocationTree::Node> rootChildNodes = parametricLocationTree.getChildNodes(rootNode);
    std::vector<ParametricLocationTree::Node> stocChildNodes;
    for (ParametricLocationTree::Node node: rootChildNodes) {
        if (node.getParametricLocation().getSourceEvent().getEventType() == EventType::General) {
            point << 2,2;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), true);
            point << 1,2;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), true);
            point << 1,3;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), false);
            point << 1,0.5;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), false);
        } else {
            point << 8,6;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), true);
            point << 8,4;
            ASSERT_EQ(node.getRegion().contains(Point<double>(point)), false);
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

TEST_F(ParametricLocationTreeTest, CandidateRegionsForTimeTest) {
    parametricLocationTree.updateRegions();
    ASSERT_EQ(parametricLocationTree.getCandidateLocationsForTime(3).size(), 3);
    ASSERT_EQ(parametricLocationTree.getCandidateLocationsForTime(6).size(), 4);
    ASSERT_EQ(parametricLocationTree.getCandidateLocationsForTime(9).size(), 5);
    ASSERT_EQ(parametricLocationTree.getCandidateLocationsForTime(11).size(), 0);
}