#include "gtest/gtest.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Event.h"

using namespace hpnmg;
using namespace std;

TEST(ParametricLocation, ConstructorThreeArgumentsTest) {
    ParametricLocation parametricLocation(2, 2, 2);

    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Root);
    ASSERT_EQ(parametricLocation.getDimension(), 3);
}

TEST(ParametricLocation, ConstructorFourArgumentsTest) {
    Event event(EventType::Immediate, vector<double>(3), 3);
    ParametricLocation parametricLocation(2, 2, 3, event);

    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Immediate);
    ASSERT_EQ(parametricLocation.getDimension(), 4);
}

TEST(ParametricLocation, ConstructorFiveArgumentsTest) {
    Event event(EventType::Immediate, vector<double>(3), 3);
    ParametricLocation parametricLocation(2, 2, 3, event, vector<vector<vector<double>>>{{{3}, {7}}},
                                          vector<vector<vector<double>>>{{{7}, {9}}});

    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Immediate);
    ASSERT_EQ(parametricLocation.getDimension(), 4);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundLeft().at(0).at(0).at(0), 3);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundLeft().at(0).at(1).at(0), 7);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundRight().at(0).at(0).at(0), 7);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundRight().at(0).at(1).at(0), 9);
}

TEST(ParametricLocation, ConstructorCustomVectors) {
    ParametricLocation parametricLocation(vector<int>{0, 0}, vector<vector<double>>{{0, 10}}, vector<double>{1}, 1,
            Event(EventType::Timed, vector<double>{-1}, 0), vector<vector<vector<double>>> {{{7.5}}},
            vector<vector<vector<double>>> {{{10}}});
    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Timed);
    ASSERT_EQ(parametricLocation.getDimension(), 2);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundLeft().at(0).at(0).at(0), 7.5);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundRight().at(0).at(0).at(0), 10);
    ASSERT_EQ(parametricLocation.getDrift().at(0), 1);
}

