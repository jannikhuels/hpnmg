#include "gtest/gtest.h"
#include "datastructures/ParametricLocation.h"
#include "datastructures/Event.h"

using namespace hpnmg;

TEST(ParametricLocation, ConstructorThreeArgumentsTest) {
    ParametricLocation parametricLocation(2,2,2);

    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Root);
    ASSERT_EQ(parametricLocation.getDimension(), 3);
}

TEST(ParametricLocation, ConstructorFourArgumentsTest) {
    Event event(EventType::Immediate, std::vector<Number>(3), 3);
    ParametricLocation parametricLocation(2,2,3,event);

    ASSERT_EQ(parametricLocation.getSourceEvent().getEventType(), EventType::Immediate);
    ASSERT_EQ(parametricLocation.getDimension(), 4);
}