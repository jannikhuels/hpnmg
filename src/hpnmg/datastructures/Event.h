#pragma once

#include <vector>
#include <hpnmg/ParametricLocationTree.h>
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
        std::vector<double> generalDependencies; // factors of s_1 ... s_n
        std::vector<double> generalDependenciesNormed;
        RateDependencies rateDependencies = {0,0}; // factor + exponent of r
        double time; // scalar

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

        RateDependencies getRateDependencies() const;
        void setRateDependencies(const RateDependencies rateDependencies);

        double getTime() const;
        void setTime(double time);

        std::vector<double> getTimeVector(int dimension);

        void print() const;
    };
}
