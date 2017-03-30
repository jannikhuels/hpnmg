#include "gtest/gtest.h"
#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include "datastructures/Event.h"

using namespace hpnmg;

#define MAX_TIME 10

class ParametricLocationEmptyTreeTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ParametricLocationEmptyTreeTest() : parametricLocationTree(ParametricLocation(1,1,1),MAX_TIME) {

    }

    virtual void SetUp() {
        
    }

    virtual void TearDown() {
    
    }
};

class ParametricLocationTreeTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ParametricLocationTreeTest() : parametricLocationTree(ParametricLocation(1,1,1), MAX_TIME) {

    }

    virtual void SetUp() {

        ParametricLocationTreeNode rootNode = parametricLocationTree.getRootNode();
        ParametricLocation childNode = rootNode.second;

        Event event;
        event.setEventType(EventType::Immediate);
        childNode.setSourceEvent(event);

        parametricLocationTree.setChildNode(rootNode, childNode);

    }

    virtual void TearDown() {
    
    }
};

TEST_F(ParametricLocationEmptyTreeTest, RootNodeTest)
{
    ParametricLocationTreeNode rootNode = parametricLocationTree.getRootNode();
    ASSERT_EQ(rootNode.first, ROOT_NODE_ID);
    ASSERT_EQ(rootNode.second.getSourceEvent().getEventType(), EventType::Root);
}

TEST_F(ParametricLocationEmptyTreeTest, NoChildNodes)
{
    try {
        ParametricLocationTreeNode rootNode = parametricLocationTree.getRootNode();
        parametricLocationTree.getChildNodes(rootNode);
        FAIL() << "The root Location should not have any child nodes.";
    } catch(NoSuchNodeException e) {
        SUCCEED();
    }
}

TEST_F(ParametricLocationTreeTest, InvalidParentNode)
{
    try {
        ParametricLocation location(1,1,1);
        ParametricLocationTreeNode invalidNode(-1,location);
        parametricLocationTree.getChildNodes(invalidNode);
        FAIL() << "This Location is not initialized and an exception should be thrown.";
    } catch(InvalidParentNodeException e) {
        SUCCEED();
    }
}

TEST_F(ParametricLocationTreeTest, ChildNode)
{
    ParametricLocationTreeNode rootNode = parametricLocationTree.getRootNode();
    std::vector<ParametricLocationTreeNode> childNodes = parametricLocationTree.getChildNodes(rootNode);

    ASSERT_EQ(childNodes.size(), 1);
}

TEST_F(ParametricLocationEmptyTreeTest, IncorrectChildNode) 
{
    try {
        ParametricLocationTreeNode rootNode = parametricLocationTree.getRootNode();
        ParametricLocation childNode = rootNode.second;

        Event event;
        event.setEventType(EventType::Immediate);
        childNode.setSourceEvent(event);

        ParametricLocation location(1,1,1);
        ParametricLocationTreeNode invalidNode(-1,location);

        parametricLocationTree.setChildNode(invalidNode, childNode);
        FAIL() << "This is an Invalid Parent Node and thus no Child nodes can be added.";
    } catch(InvalidParentNodeException e) {
        SUCCEED();
    }
    
}