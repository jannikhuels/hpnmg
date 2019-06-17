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
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("example2general.xml");
    auto plt1 = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, 10);
    auto plt2 = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, 10);
    auto plt3 = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, 10);
    auto modelChecker = RegionModelChecker(*hpng, 10);
    auto plt = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, 10);
    PLTWriter{}.writePLT(plt, 10);

    auto formula = Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1));

    EXPECT_DOUBLE_EQ(1.0, modelChecker.satisfies(formula, 2));
}

TEST(RegionModelChecker, ConjunctionTest) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 10);
    auto formula = Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    ));


    EXPECT_DOUBLE_EQ(1.0, modelChecker.satisfies(formula, 0));
}
