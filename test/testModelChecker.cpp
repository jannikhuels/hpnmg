#include "gtest/gtest.h"
#include "ModelChecker.h"

using namespace std;
using namespace hpnmg;

#define MAX_TIME 10

class ModelCheckerTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ModelCheckerTest() : parametricLocationTree(
            ParametricLocation(vector<int>{1, 1}, vector<vector<double>>{vector<double>{0, 0}}, vector<double>{1}, 1,
                               Event(EventType::Root, vector<double>{0}, 0), vector<vector<vector<double>>> {{{0}}},
                               vector<vector<vector<double>>> {{{10}}}), 10) {

    }

    virtual void SetUp() {

        ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();

        ParametricLocation childNode = rootNode.getParametricLocation();

        ParametricLocationTree::Node detNode = parametricLocationTree.setChildNode(rootNode,
             ParametricLocation(vector<int>{1, 0}, vector<vector<double>>{vector<double>{0, 5}}, vector<double>{2},
                                1, Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{5}}},
                                vector<vector<vector<double>>> {{{10}}}));

        ParametricLocationTree::Node stocNode = parametricLocationTree.setChildNode(rootNode, ParametricLocation(
                vector<int>{0, 1}, vector<vector<double>>{vector<double>{1, 0}}, vector<double>{-1}, 1,
                Event(EventType::General, vector<double>{-1}, 0), vector<vector<vector<double>>> {{{0}}},
                vector<vector<vector<double>>> {{{5}}}));

        ParametricLocationTree::Node emptyNode = parametricLocationTree.setChildNode(stocNode, ParametricLocation(
                vector<int>{0, 1}, vector<vector<double>>{vector<double>{2, 0}}, vector<double>{0}, 1,
                Event(EventType::General, vector<double>{-2}, 0), vector<vector<vector<double>>> {{{0}}},
                vector<vector<vector<double>>> {{{2.5}}}));

        ParametricLocationTree::Node timedAfterStochastic = parametricLocationTree.setChildNode(stocNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{2, -5}}, vector<double>{0}, 1,
                                 Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{2.5}}},
                                 vector<vector<vector<double>>> {{{5}}}));

        ParametricLocationTree::Node timedAfterEmpty = parametricLocationTree.setChildNode(emptyNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{0, 0}}, vector<double>{0}, 1,
                                 Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{0}}},
                                 vector<vector<vector<double>>> {{{2.5}}}));

        ParametricLocationTree::Node stochasticAfterTimed = parametricLocationTree.setChildNode(detNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{2, -5}}, vector<double>{0}, 1,
                                 Event(EventType::General, vector<double>{-1}, 0), vector<vector<vector<double>>> {{{5}}},
                                 vector<vector<vector<double>>> {{{7.5}}}));

        ParametricLocationTree::Node fullNode = parametricLocationTree.setChildNode(detNode, ParametricLocation(
                vector<int>{1, 0}, vector<vector<double>>{vector<double>{0, 10}}, vector<double>{0}, 1,
                Event(EventType::Timed, vector<double>{0}, 7.5), vector<vector<vector<double>>> {{{7.5}}},
                vector<vector<vector<double>>> {{{10}}}));

        ParametricLocationTree::Node stochasticAfterFull = parametricLocationTree.setChildNode(fullNode,
               ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{0, 10}}, vector<double>{0}, 1,
                                  Event(EventType::Timed, vector<double>{-1}, 0), vector<vector<vector<double>>> {{{7.5}}},
                                  vector<vector<vector<double>>> {{{10}}}));
    }

    virtual void TearDown() {

    }
};

TEST_F(ModelCheckerTest, DiscreteFormulaTest) {
    parametricLocationTree.updateRegions();
    vector<carl::Interval<double>> oneIntervalSet = ModelChecker::discreteFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 1, 3);

    ASSERT_EQ(oneIntervalSet.size(), 1);
    ASSERT_EQ(oneIntervalSet[0].lower(), 3);
    ASSERT_EQ(oneIntervalSet[0].upper(), 10);

    vector<carl::Interval<double>> threeIntervalSets = ModelChecker::discreteFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 0, 6);

    ASSERT_EQ(threeIntervalSets.size(), 3);

    sort(threeIntervalSets.begin(), threeIntervalSets.end(), ModelChecker::sortIntervals);

    ASSERT_EQ(threeIntervalSets[0].lower(), 0);
    ASSERT_EQ(threeIntervalSets[0].upper(), 2.5);
    ASSERT_EQ(threeIntervalSets[1].lower(), 2.5);
    ASSERT_EQ(threeIntervalSets[1].upper(), 5);
    ASSERT_EQ(threeIntervalSets[2].lower(), 5);
    ASSERT_EQ(threeIntervalSets[2].upper(), 6);
}


TEST_F(ModelCheckerTest, ContinuousFormulaTest) {
    parametricLocationTree.updateRegions();
    vector<carl::Interval<double>> intervalSets = ModelChecker::continuousFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 4, 3);

    ASSERT_EQ(intervalSets.size(), 3);

    sort(intervalSets.begin(), intervalSets.end(), ModelChecker::sortIntervals);

    ASSERT_EQ(intervalSets[0].lower(), 0);
    ASSERT_EQ(intervalSets[0].upper(), 1.5);
    ASSERT_EQ(intervalSets[1].lower(), 1.5);
    ASSERT_EQ(intervalSets[1].upper(), 3);
    ASSERT_EQ(intervalSets[2].lower(), 3);
    ASSERT_EQ(intervalSets[2].upper(), 10);
}
