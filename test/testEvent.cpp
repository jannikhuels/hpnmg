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
    Event event(EventType::Timed, std::vector<Number>(3),3);
    ASSERT_EQ(event.getEventType(), EventType::Timed);
    ASSERT_EQ(event.getTime(), 3);
    ASSERT_EQ(event.getGeneralDependencies().size(), 3);
}