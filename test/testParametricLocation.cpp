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
            Event(EventType::Timed, vector<double>{1}, 0), vector<vector<vector<double>>> {{{7.5}}},
            vector<vector<vector<double>>> {{{10}}});
    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Timed);
    ASSERT_EQ(parametricLocation.getDimension(), 2);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundLeft().at(0).at(0).at(0), 7.5);
    ASSERT_EQ(parametricLocation.getGeneralIntervalBoundRight().at(0).at(0).at(0), 10);
    ASSERT_EQ(parametricLocation.getDrift().at(0), 1);
}

TEST(ParametricLocation, EntryTime) {
    Event event1(EventType::Immediate, vector<double>(2), 8);
    ParametricLocation parametricLocation1(2, 2, 3, event1, vector<vector<vector<double>>>{{{3}, {7}}},
                                          vector<vector<vector<double>>>{{{7}, {9}}});
    parametricLocation1.setGeneralTransitionsFired({0,0});
    ASSERT_EQ(parametricLocation1.getEarliestEntryTime(),8);
    ASSERT_EQ(parametricLocation1.getLatestEntryTime(),8);

    Event event2(EventType::Immediate, vector<double>{1,1}, 8);
    ParametricLocation parametricLocation2(2, 2, 3, event2, vector<vector<vector<double>>>{{{3}, {7}}},
                                          vector<vector<vector<double>>>{{{7}, {9}}});
    parametricLocation2.setGeneralTransitionsFired({0,0});
    ASSERT_EQ(parametricLocation2.getEarliestEntryTime(),18);
    ASSERT_EQ(parametricLocation2.getLatestEntryTime(),24);

    Event event3(EventType::Immediate, vector<double>{1,5}, 8);
    ParametricLocation parametricLocation3(2, 2, 3, event3, vector<vector<vector<double>>>{{{3}, {7}}},
                                           vector<vector<vector<double>>>{{{7}, {9}}});
    parametricLocation3.setGeneralTransitionsFired({0,0});
    ASSERT_EQ(parametricLocation3.getEarliestEntryTime(),46); // 8 + 3 + 35
    ASSERT_EQ(parametricLocation3.getLatestEntryTime(),60); // 8 + 7 + 45

    Event event4(EventType::Immediate, vector<double>{3,-1}, 8);
    ParametricLocation parametricLocation4(2, 2, 3, event4, vector<vector<vector<double>>>{{{3}, {7}}},
                                           vector<vector<vector<double>>>{{{7}, {9}}});
    parametricLocation4.setGeneralTransitionsFired({0,0});
    ASSERT_EQ(parametricLocation4.getEarliestEntryTime(),8); // 8 + 9 - 9
    ASSERT_EQ(parametricLocation4.getLatestEntryTime(),22); // 8 + 21 - 7
    
    Event event5(EventType::Timed, vector<double>{0,0,2},7);
    ParametricLocation parametricLocation5(2,2,3,event5,vector<vector<vector<double>>>{{{2},{4,0,-1}},{{3,1}}},vector<vector<vector<double>>>{{{3},{5,0,1}},{{4,1}}});
    parametricLocation5.setGeneralTransitionsFired({0,1,0});
    EXPECT_EQ(1, parametricLocation5.getEarliestEntryTime());
}

