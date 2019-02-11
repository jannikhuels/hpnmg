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
        std::vector<double> generalDependenciesNormed;
        double time;

    public:
        Event(EventType type, std::vector<double> generalDependencies, double time);
        Event();
        Event(int numberOfGeneralTransitions);
        Event(const Event &event);

        EventType getEventType() const;
        void setEventType(const EventType &eventType);

        std::vector<double> getGeneralDependencies() const;
        void setGeneralDependencies(const std::vector<double> generalDependencies);

        std::vector<double> getGeneralDependenciesNormed() const;
        void setGeneralDependenciesNormed(const std::vector<double> generalDependencies);

        double getTime() const;
        void setTime(double time);

        std::vector<double> getTimeVector(int dimension);

        void print() const;
    };
}
