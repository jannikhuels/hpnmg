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

TEST(RegionModelChecker, DiscreteAtomicPropertyTest1GT) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50);
    auto formula = Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1));

    const auto result = modelChecker.satisfies(formula, 1);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(1)) = 1 - (0.5 * (Erf((1 + 5) / sqrt(18)) + erf((1 - 5) / sqrt(18)))) ~ 0.931539
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

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), 20);
    EXPECT_NEAR(1.0, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)), 20);
    EXPECT_NEAR(0.0, result.first, result.second);
}

TEST(RegionModelChecker, ConjunctionTest) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 10);
    auto formula = Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    ));


    const auto result = modelChecker.satisfies(formula, 0);
    EXPECT_NEAR(1.0, result.first, result.second);
}
