#include "NewParametricLocation.h"

#include <algorithm>
#include <map>
#include <unordered_map>

namespace hpnmg {
    NewParametricLocation::NewParametricLocation(const hpnmg::ParametricLocation &legacyLocation) : legacyLocation(legacyLocation) {}

    std::map<FiringIndex, FiringInterval> NewParametricLocation::getFiringIntervals() const {
        const auto &generalTransitionsFired = legacyLocation.getGeneralTransitionsFired();

        /** Maps firing order indexes to correct FiringIndex instances */
        std::vector<FiringIndex> firingIndexes(generalTransitionsFired.size());

        std::unordered_map<int, int> generalTransitionFiringCount{};
        auto generalTransitionsFiredIndex = 0;
        std::for_each(
            firingIndexes.begin(),
            firingIndexes.end(),
            [&generalTransitionsFired, &generalTransitionsFiredIndex, &generalTransitionFiringCount](auto &firingIndex) {
                const auto generalTransition = generalTransitionsFired[generalTransitionsFiredIndex++];
                firingIndex.transition = generalTransition;
                firingIndex.firing = generalTransitionFiringCount[generalTransition]++;
            }
        );

        auto leftBoundaries = legacyLocation.getGeneralIntervalBoundLeft();
        auto rightBoundaries = legacyLocation.getGeneralIntervalBoundRight();

        std::map<FiringIndex, FiringInterval> result{};
        for (auto transition = 0; transition < leftBoundaries.size(); ++transition) {
                const auto firings = leftBoundaries[transition];
                for (auto firing = 0; firing < firings.size(); ++firing) {
                        const auto leftDependencies = firings[firing];
                        const auto rightDependencies = rightBoundaries[transition][firing];

                        auto lower = DependenciesWithTime{};
                        auto upper = DependenciesWithTime{};

                        lower.time = leftDependencies[0];
                        upper.time = rightDependencies[0];
                        for (auto dependencyIndex = 1; dependencyIndex < leftDependencies.size(); ++dependencyIndex) {
                                const auto firingIndex = firingIndexes[dependencyIndex - 1];
                                lower.dependencies.emplace(firingIndex, leftDependencies[dependencyIndex]);
                                upper.dependencies.emplace(firingIndex, rightDependencies[dependencyIndex]);
                        }
                }
        }
        /*
         * Create all intervals for GTs that are currently enabled.
         */
//        for (int j = 0; j < occurings.size(); j++) {
//            for (int i = counter[j]; i < occurings[j]; i++) {
//                int firing = i;
//                if (j < legacyLocation.getGeneralIntervalBoundLeft().size() && firing < legacyLocation.getGeneralIntervalBoundLeft()[j].size()) {
//                    bool enablingTimeGreaterZero = false;
//                    for (int l = 0; l < legacyLocation.getGeneralIntervalBoundLeft()[j][firing].size(); l++) {
//                        if (legacyLocation.getGeneralIntervalBoundLeft()[j][firing][l] > 0)
//                            enablingTimeGreaterZero = true;
//                    }
//                    if (legacyLocation.getGeneralTransitionsEnabled()[j] || enablingTimeGreaterZero) {
//                        result.push_back({j, std::pair<std::vector<double>, std::vector<double>>(
//                                fillVector(leftBoundaries[j][firing], dim),
//                                fillVector(rightBoundaries[j][firing], dim))});
//                        continue;
//                    }
//                }
//                result.push_back({j, std::pair<std::vector<double>, std::vector<double>>(zero, mTime)});
//            }
//        }
        return {};
    }
}
