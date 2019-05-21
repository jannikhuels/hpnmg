#include "gtest/gtest.h"

#include "modelchecker/Conjunction.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
#include "modelchecker/RegionModelChecker.h"
#include "ParametricLocationTree.h"
#include "ParseHybridPetrinet.h"
#include "ReadHybridPetrinet.h"

#include <chrono>
#include <memory>

using namespace hpnmg;

TEST(RegionModelChecker, DiscreteAtomicPropertyTest) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 20);
    auto formula = Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1));

    EXPECT_DOUBLE_EQ(1.0, modelChecker.satisfies(formula, 0));
}

TEST(RegionModelChecker, ConjunctionTest) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 20);
    auto formula = Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    ));


    EXPECT_DOUBLE_EQ(1.0, modelChecker.satisfies(formula, 0));
}
