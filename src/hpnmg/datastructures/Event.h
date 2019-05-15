#pragma once

#include <vector>
#include "representations/GeometricObject.h"

namespace hpnmg {

    enum EventType {
        Immediate = 1,
        General = 2,
        Timed = 3,
        Continuous = 4,
        Root = 5,
    };

    class Event {
    private:
        EventType type;
        std::vector<double> generalDependencies;
        double time;
        std::string id;

    public:
        Event(EventType type, std::vector<double> generalDependencies, double time, std::string id = "");
        Event();
        Event(int numberOfGeneralTransitions);
        Event(const Event &event);

        EventType getEventType() const;
        void setEventType(const EventType &eventType);

        std::vector<double> getGeneralDependencies() const;
        void setGeneralDependencies(const std::vector<double> generalDependencies);

        double getTime() const;
        void setTime(double time);

        std::string getId() const;
        void setId(std::string id);

        std::vector<double> getTimeVector(int dimension);

        void print() const;
    };
}
