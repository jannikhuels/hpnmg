#include "gtest/gtest.h"

#include "modelchecker/Conjunction.h"
#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Negation.h"
#include "modelchecker/RegionModelChecker.h"
#include "modelchecker/Until.h"
#include "ParametricLocationTree.h"
#include "ReadHybridPetrinet.h"
#include "PLTWriter.h"
#include <chrono>
#include <memory>

using namespace hpnmg;

TEST(RegionModelChecker, StandardExampleUntil){

    auto	reader	       = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("example.xml");

    auto formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("pd2", 0)), 10));
    auto modelChecker = RegionModelChecker(*hybridPetrinet, 10, 1, 2, formula);
    pair<double,double> result1 = modelChecker.satisfies(formula, 2);
    modelChecker = RegionModelChecker(*hybridPetrinet, 10);
    pair<double,double> result2 = modelChecker.satisfies(formula, 2);

    ASSERT_EQ(result1.first, result2.first);


    formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("pd1", 0)), 10));
    modelChecker = RegionModelChecker(*hybridPetrinet, 10, 1, 2, formula);
    result1 = modelChecker.satisfies(formula, 2);
    modelChecker = RegionModelChecker(*hybridPetrinet, 10);
    result2 = modelChecker.satisfies(formula, 2);

    ASSERT_EQ(result1.first, result2.first);

    formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 5)), 10));
    modelChecker = RegionModelChecker(*hybridPetrinet, 10, 1, 2, formula);
    result1 = modelChecker.satisfies(formula, 2);
    modelChecker = RegionModelChecker(*hybridPetrinet, 10);
    result2 = modelChecker.satisfies(formula, 2);

    ASSERT_EQ(result1.first, result2.first);

}

TEST(RegionModelChecker, testUntilBatteryCharging){

    auto	reader	       = new ReadHybridPetrinet();
    auto	hybridPetrinet = reader->readHybridPetrinet("ev_charging.xml");

    //Formeln
    auto formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("unstable_grid", 1)), 10));
    auto modelChecker = RegionModelChecker(*hybridPetrinet, 10, 1, 2, formula);
    pair<double,double> result1 = modelChecker.satisfies(formula, 2);
    modelChecker = RegionModelChecker(*hybridPetrinet, 10);
    pair<double,double> result2 = modelChecker.satisfies(formula, 2);
    ASSERT_EQ(result1.first, result2.first);

    formula = Formula(std::make_shared<Until>(
        Formula(std::make_shared<True>()), Formula(std::make_shared<DiscreteAtomicProperty>("unstable_grid", 1)), 10));
    modelChecker = RegionModelChecker(*hybridPetrinet, 10, 1, 2, formula);
    result1 = modelChecker.satisfies(formula, 2);
    modelChecker = RegionModelChecker(*hybridPetrinet, 10);
    result2 = modelChecker.satisfies(formula, 2);
    ASSERT_EQ(result1.first, result2.first);
}


TEST(RegionModelChecker, testUntilPLT) {
    const double maxTime = 20;
    // TG1: uniform distribution over [0, 10]
    auto hpn = ReadHybridPetrinet{}.readHybridPetrinet("norep_1_1.xml");
    auto modelChecker = RegionModelChecker(*hpn, maxTime, 1, 0, Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)),
        Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)))),
        maxTime
    )));

    // Place is empty until it finally is not empty anymore.
    auto result = modelChecker.satisfies(Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)),
        Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 0)))),
        maxTime
    )), 0);
    EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    // The place's level does not exceed 2 until the input transition is finally disabled with the level still being low
    modelChecker = RegionModelChecker(*hpn, maxTime, 1, 2, Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)))),
        maxTime)));

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
    )), 2);
    EXPECT_NEAR(0.4, round(result.first * 10) / 10, result.second);

    // The place's level does not exceed 2 until the input transition is finally disabled
    modelChecker = RegionModelChecker(*hpn, maxTime, 1, 4, Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0))
        )),
        maxTime
    )));
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
    modelChecker = RegionModelChecker(*hpn, maxTime, 1, 6, Formula(std::make_shared<Until>(
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0))
        )),
        maxTime
    )));
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
    modelChecker = RegionModelChecker(*hpn, maxTime, 1, 7, Formula(std::make_shared<Until>(
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), // input disabled
            Formula(std::make_shared<DiscreteAtomicProperty>("pout1", 1)) // no output disabled
        )),
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        maxTime
    )));
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
    modelChecker = RegionModelChecker(*hpn, maxTime, 1, 8, Formula(std::make_shared<Until>(
        Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 0)), // input disabled
            Formula(std::make_shared<DiscreteAtomicProperty>("pout1", 1)) // no output disabled
        )),
        Formula(std::make_shared<ContinuousAtomicProperty>("pc1", 2)),
        maxTime
    )));
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
