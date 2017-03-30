#pragma once

#include <vector>
#include "representations/GeometricObject.h"

namespace hpnmg {

    using Number = double;

    enum EventType {
        Immediate = 1,
        General = 2,
        Timed = 3,
        Continuous = 4,
        Root = 5
    };

    class Event {
    private:
        EventType type;
        std::vector<Number> generalDependencies;
        int time;

    public:
        Event(EventType type, std::vector<Number> generalDependencies, int time);
        Event();

        EventType getEventType() const;
        void setEventType(const EventType &eventType);

        std::vector<Number> getGeneralDependencies() const;
        void setGeneralDependencies(const std::vector<Number> generalDependencies);

        int getTime() const;
        void setTime(int time);
    };
}
