#include "gtest/gtest.h"
#include "datastructures/Event.h"

using namespace hpnmg;

TEST(Event, DefaultConstructor)
{
    Event event;
    ASSERT_EQ(event.getEventType(), EventType::Root);
    ASSERT_EQ(event.getTime(), 0);
    ASSERT_EQ(event.getGeneralDependencies().size(), 0);
}

TEST(Event, CustomConstructor)
{
    Event event(EventType::Timed, std::vector<double>(3),3);
    ASSERT_EQ(event.getEventType(), EventType::Timed);
    ASSERT_EQ(event.getTime(), 3);
    ASSERT_EQ(event.getGeneralDependencies().size(), 3);
}

TEST(Event, SetterTest)
{
    Event event;
    event.setEventType(EventType::Immediate);
    event.setGeneralDependencies(vector<double> {-1});
    event.setTime(42);

    ASSERT_EQ(event.getEventType(), EventType::Immediate);
    ASSERT_EQ(event.getTime(), 42);
    ASSERT_EQ(event.getGeneralDependencies().size(), 1);
    ASSERT_EQ(event.getGeneralDependencies()[0], -1);
}