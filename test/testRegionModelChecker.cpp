#include "gtest/gtest.h"

#include "modelchecker/Conjunction.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
#include "modelchecker/RegionModelChecker.h"
#include "ParametricLocationTree.h"
#include "ParseHybridPetrinet.h"
#include "ReadHybridPetrinet.h"

#include "PLTWriter.h"

#include <chrono>
#include <memory>

using namespace hpnmg;

TEST(RegionModelChecker, ContinuousAtomicPropertyTest1GTFoldedNormal) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50);

    auto result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 0);
    // Place is empty at t = 0
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 3)), 3);
    // out-transition is deterministically disabled at t=5, so the place's level cannot exceed t' at time t'<=5
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 3);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(2.5) = 0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))) ~ 0.196119
    EXPECT_NEAR(0.196119, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 7)), 10);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(6)) = 1 - (0.5 * (erf((6 + 5) / sqrt(18)) + erf((6 - 5) / sqrt(18)))) ~ 0.630436
    EXPECT_NEAR(0.630436, result.first, result.second);
}

TEST(RegionModelChecker, ContinuousAtomicPropertyTest1GTUniform) {
    // TG1: uniform distribution over [0, 10]
    auto hpn = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_1.xml");
    auto modelChecker = RegionModelChecker(*hpn, 20);

    auto result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 0);
    // Place is empty at t = 0
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 5);
    EXPECT_NEAR(0.45, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 5)), 10);
    // one out-transition is deterministically disabled at t=12, so the place's level cannot exceed t'/2 at time t'<=12
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 10);
    EXPECT_NEAR(0.7, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 15);
    EXPECT_NEAR(0.675, result.first, result.second);
}

TEST(RegionModelChecker, DiscreteAtomicPropertyTest1GT) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50);
    auto formula = Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1));

    const auto result = modelChecker.satisfies(formula, 1);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(1)) = 1 - (0.5 * (erf((1 + 5) / sqrt(18)) + erf((1 - 5) / sqrt(18)))) ~ 0.931539
    EXPECT_NEAR(0.931539, result.first, result.second);
}

TEST(RegionModelChecker, DiscreteAtomicPropertyTest2GT) {
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_2.xml");
    auto modelChecker = RegionModelChecker(*hpng, 20);

    auto result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 2)), 0);
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), 5);
    EXPECT_NEAR(0.125, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)), 10);
    EXPECT_NEAR(0.5, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), 13);
    EXPECT_NEAR(0.755, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)), 13);
    EXPECT_NEAR(0.245, result.first, result.second);
}

TEST(RegionModelChecker, ConjunctionTest) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50);

    auto result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 0);
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(2.5)) = 1 - (0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18)))) ~ 0.803881
    EXPECT_NEAR(0.803881, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(2.5) = 0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))) ~ 0.196119
    EXPECT_NEAR(0.196119, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 6);
    EXPECT_NEAR(0.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 0))
    )), 6);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(6) = 0.5 * (erf((6 + 5) / sqrt(18)) + erf((6 - 5) / sqrt(18))) ~ 0.499571
    EXPECT_NEAR(0.630436, result.first, result.second);
}
