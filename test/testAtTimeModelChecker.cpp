#include "gtest/gtest.h"

#include "modelchecker/Conjunction.h"
#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Negation.h"
#include "modelchecker/RegionModelChecker.h"
#include "modelchecker/Until.h"
#include "ParametricLocationTree.h"
#include "PropertyBasedPLTBuilder.h"
#include "ReadHybridPetrinet.h"

#include "PLTWriter.h"

#include <chrono>
#include <memory>

using namespace hpnmg;


TEST(RegionModelChecker, ContinuousAtomicPropertyTest1GTFoldedNormalAtTime) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 1, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 0);
    // Place is empty at t = 0
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 7, 1);
    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 3)), 3);
    // out-transition is deterministically disabled at t=5, so the place's level cannot exceed t' at time t'<=5
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 3);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(2.5) = 0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))) ~ 0.196119
    EXPECT_NEAR(0.1961187156378668902015554951380463273568632340661803, result.first, result.second);

    modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 10, 1);
    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 7)), 10);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(6)) = (0.5 * (erf((6 + 5) / sqrt(18)) + erf((6 - 5) / sqrt(18)))) ~ 0.630436
     EXPECT_NEAR(0.6304357934282712096662251163331139441485145682519407, result.first, result.second);
}
/*
TEST(RegionModelChecker, ContinuousAtomicPropertyTest1GTUniformAtTime) {
    // TG1: uniform distribution over [0, 10]
    auto hpn = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_1.xml");
    auto modelChecker = RegionModelChecker(*hpn, 20, 15, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 0);
    // Place is empty at t = 0
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 5);
    EXPECT_NEAR(0.45, round(result.first*100)/100, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 5)), 10);
    // one out-transition is deterministically disabled at t=12, so the place's level cannot exceed t'/2 at time t'<=12
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)), 10);
    EXPECT_NEAR(0.7, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)), 15);
    EXPECT_NEAR(0.675, round(result.first*1000)/1000, result.second);
}

TEST(RegionModelChecker, ContinuousAtomicPropertyTest2ConflictGTUniformAtTime) {
    // TG1 and TG2: uniform distribution over [0, 100]
    auto hpn = ReadHybridPetrinet{}.readHybridPetrinet("heated_tank_minimized.xml");
    auto modelChecker = RegionModelChecker(*hpn, 1000, 10, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("H", 100)), 0);
    // Place is at level 100 at time 0
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<ContinuousAtomicProperty>("H", 97)), 10);
    EXPECT_NEAR(0.00202083, round(result.first*100000000)/100000000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("H", 103)))), 10);
    EXPECT_NEAR(0.0, round(result.first*10)/10, result.second);
}
*/
TEST(RegionModelChecker, DiscreteAtomicPropertyTest1GTAtTime) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 1 ,1);
    auto formula = Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1));

    const auto result = modelChecker.satisfies(formula, 1);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(1)) = 1 - (0.5 * (erf((1 + 5) / sqrt(18)) + erf((1 - 5) / sqrt(18)))) ~ 0.931539
    EXPECT_NEAR(0.9315389122223113373664299671527104472835967852691848, result.first, result.second);
}
/*
TEST(RegionModelChecker, DiscreteAtomicPropertyTest2GTAtTime) {
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_2.xml");
    auto modelChecker = RegionModelChecker(*hpng, 20, 13, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 2)), 0);
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), 5);
    EXPECT_NEAR(0.125, round(result.first*1000)/1000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)), 10);
    EXPECT_NEAR(0.5, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), 13);
    EXPECT_NEAR(0.755, round(result.first*1000)/1000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)), 13);
    EXPECT_NEAR(0.245, round(result.first*1000)/1000, result.second);
}

TEST(RegionModelChecker, ConjunctionTestAtTime) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 6, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 0);
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(2.5)) = 1 - (0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18)))) ~ 0.803881
    EXPECT_NEAR(0.8038812843621331097984445048619536726431367659338196, result.first, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(2.5) = 0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))) ~ 0.196119
    EXPECT_NEAR(0.1961187156378668902015554951380463273568632340661803, result.first, result.second);

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
    EXPECT_NEAR(0.6304357934282712096662251163331139441485145682519407, result.first, result.second);
}

TEST(RegionModelChecker, DiscreteAtomicPropertyNegationTest1GTAtTime) {
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("example.xml");
    auto modelChecker = RegionModelChecker(*hpng, 50, 1, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)))), 1);

    // folded normal distribution with mu = 5 and sigma = 3
    // (cdf(1)) = (0.5 * (erf((1 + 5) / sqrt(18)) + erf((1 - 5) / sqrt(18)))) ~ 1 - 0.931539
    EXPECT_NEAR(0.0684610877776886626335700328472895527164032147308151, result.first, result.second);
}

TEST(RegionModelChecker, ContinuousAtomicPropertyNegationTest1GTFoldedNormalAtTime) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 10, 1);

    // This test is especially important, as it also covers the openFacet problem.
    auto result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)))), 0);
    // Place is not empty at t = 0
    EXPECT_NEAR(0.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 3)))), 3);
    // out-transition is deterministically disabled at t=5, so the place's level cannot exceed t' at time t'<=5
    EXPECT_NEAR(0.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)))), 3);
    // folded normal distribution with mu = 5 and sigma = 3
    // cdf(2.5) = 0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))) ~ 1 - 0.1961
    EXPECT_NEAR(0.8039, round(result.first * 10000) / 10000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 7)))), 10);
    // folded normal distribution with mu = 5 and sigma = 3
    // (1 - cdf(6)) = (1 - 0.5 * (erf((6 + 5) / sqrt(18)) + erf((6 - 5) / sqrt(18)))) ~ 1 - 0.630436
    EXPECT_NEAR(0.369564, round(result.first * 1000000) / 1000000, result.second);
}

TEST(RegionModelChecker, ConjunctionNegationTestAtTime) {
    auto modelChecker = RegionModelChecker(*ReadHybridPetrinet{}.readHybridPetrinet("example.xml"), 50, 6, 1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )))), 0);
    EXPECT_NEAR(0.0, round(result.first * 10000) / 10000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )))), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // 1 - (1 - cdf(2.5)) = 1 - (1 - (0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18))))) ~ 1 - 0.803881
    EXPECT_NEAR(0.1961, round(result.first * 10000) / 10000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )))), 2.5);
    // folded normal distribution with mu = 5 and sigma = 3
    // 1 - cdf(2.5) = 1 - (0.5 * (erf((2.5 + 5) / sqrt(18)) + erf((2.5 - 5) / sqrt(18)))) ~ 1 - 0.196119
    EXPECT_NEAR(0.8039, round(result.first * 10000) / 10000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 1)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 1))
    )))), 6);
    EXPECT_NEAR(1.0, round(result.first * 10000) / 10000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<Conjunction>(
        Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)),
        Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 0))
    )))), 6);
    // folded normal distribution with mu = 5 and sigma = 3
    // 1 - cdf(6) = 1 - (0.5 * (erf((6 + 5) / sqrt(18)) + erf((6 - 5) / sqrt(18)))) ~ 1 - 0.630436
    EXPECT_NEAR(0.3696, round(result.first * 10000) / 10000, result.second);
}

TEST(RegionModelChecker, DiscreteAtomicPropertyNegationTest2GTAtTime) {
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_2.xml");
    auto modelChecker = RegionModelChecker(*hpng, 20, 13 ,1);

    auto result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 2)))), 0);
    EXPECT_NEAR(0.0, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)))), 5);
    EXPECT_NEAR(0.875, round(result.first*1000)/1000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)))), 10);
    EXPECT_NEAR(0.5, round(result.first*10)/10, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)))), 13);
    EXPECT_NEAR(0.245, round(result.first*1000)/1000, result.second);

    result = modelChecker.satisfies(Formula(std::make_shared<Negation>(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 1)))), 13);
    EXPECT_NEAR(0.755, round(result.first*1000)/1000, result.second);
}

//until needs to evaluate the whole path up to the end. Hence the property based PLT builder cant be used (yet).
TEST(RegionModelChecker, UntilUniformAtTime) {
    const double maxTime = 20;
    // TG1: uniform distribution over [0, 10]
    auto hpn = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_1.xml");
    auto modelChecker = RegionModelChecker(*hpn, maxTime);

    // Place is empty until it finally is not empty anymore.
    auto result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)),
        Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)))),
        maxTime
    )), 0);
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    // The place's level does not exceed 2 until the input transition is finally disabled with the level still being low
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0))
        )),
        maxTime
    )), 2);
    EXPECT_NEAR(0.4, round(result.first * 10) / 10, result.second);
    // The place's level does not exceed 2 until the input transition is finally disabled
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0))
        )),
        maxTime
    )), 4);
    EXPECT_NEAR(0.4, round(result.first * 10) / 10, result.second);
    // The place's level does not exceed 2 until the input transition is finally disabled
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0))
        )),
        maxTime
    )), 6);
    EXPECT_NEAR(0.5, round(result.first * 10) / 10, result.second);

    // The place is actively being emptied by two pumps below level 2
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), // input disabled
            Formula(std::make_shared<DiscreteAtomicProperty>("pout1", 1)) // no output disabled
        )),
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        maxTime
    )), 6);
    EXPECT_NEAR(0.6, round(result.first * 10) / 10, result.second);
    // The place is actively being emptied by two pumps below level 2
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), // input disabled
            Formula(std::make_shared<DiscreteAtomicProperty>("pout1", 1)) // no output disabled
        )),
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        maxTime
    )), 7);
    EXPECT_NEAR(0.7, round(result.first * 10) / 10, result.second);
    // The place is actively being emptied by two pumps below level 2
    result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), // input disabled
            Formula(std::make_shared<DiscreteAtomicProperty>("pout1", 1)) // no output disabled
        )),
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        maxTime
    )), 8);
    EXPECT_NEAR(0.8, round(result.first * 10) / 10, result.second);
}
*/
