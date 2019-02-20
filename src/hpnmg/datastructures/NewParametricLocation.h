#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "ParametricLocation.h"


namespace hpnmg {
    struct FiringIndex {
        FiringIndex(int transition, int firing) : transition(transition), firing(firing) {};

        int transition = 0;
        int firing = 0;
    };

    struct DependenciesWithTime {
        DependenciesWithTime() = default;
        DependenciesWithTime(std::map<FiringIndex, double> dependencies, double time) : dependencies(std::move(dependencies)), time(time) {};

        std::map<FiringIndex, double> dependencies = {};
        double time = 0;
    };

    struct FiringInterval {
        FiringInterval(DependenciesWithTime lower, DependenciesWithTime upper) : lower(std::move(lower)), upper(std::move(upper)) {};

        DependenciesWithTime lower = {{}, 0};
        DependenciesWithTime upper = {{}, 0};
    };

    class NewParametricLocation {
    public:
        explicit NewParametricLocation(const ParametricLocation& legacyLocation);

        const ParametricLocation &getLegacyLocation() { return legacyLocation;}

        std::map<FiringIndex, FiringInterval> getFiringIntervals() const;

    private:
        ParametricLocation legacyLocation;
    };
}
