#include <gtest/gtest.h>
#include <hpnmg/helper/LinearRestriction.h>
#include "helper/LinearRestriction.h"

using namespace hpnmg;

TEST(LinearRestrictionTest, SimpleRestriction) {
    LinearRestriction lr = LinearRestriction({0,0,0}, {4.5,-3,-2}, {0,3,1});
    ASSERT_EQ(lr.solutionOne.dependencyIndex, 1);
    ASSERT_EQ(lr.solutionOne.equation[0], 2.25);
    ASSERT_EQ(lr.solutionOne.equation[1], -1.5);
    ASSERT_EQ(lr.solutionOne.isUpper, true);
    ASSERT_EQ(lr.solutionOne.isDecidable(), false);

    ASSERT_EQ(lr.solutionTwo.dependencyIndex, 1);
    ASSERT_EQ(lr.solutionTwo.equation[0], 1.5);
    ASSERT_EQ(lr.solutionTwo.equation[1], -2);
    ASSERT_EQ(lr.solutionTwo.isUpper, false);
    ASSERT_EQ(lr.solutionTwo.isDecidable(), false);
}

TEST(LinearRestrictionTest, OneRestriction) {
    LinearRestriction lr = LinearRestriction({0,0,0}, {10,0,0}, {1.5,-2,0});
    ASSERT_EQ(true, lr.solutionOne.alwaysTrue);
    ASSERT_EQ(0, lr.solutionTwo.dependencyIndex);
    ASSERT_EQ(true, lr.solutionTwo.isUpper);
}

TEST(LinearRestrictionTest, TwoRestrictions) {
    LinearRestriction lr = LinearRestriction({0,0,0}, {0,-3,0}, {10,0,0});
    ASSERT_EQ(true, lr.solutionOne.isUpper);
    ASSERT_EQ(0, lr.solutionOne.equation[0]);
    ASSERT_EQ(0, lr.solutionTwo.dependencyIndex);
    ASSERT_EQ(false, lr.solutionTwo.isUpper);
}

TEST(LinearRestrictionTest, TwoRestrictionsFourDimension) {
    LinearRestriction lr = LinearRestriction({0,0,0,0,0}, {3,-1,-1,-1,0}, {10,0,0,0,0});
    ASSERT_EQ(true, lr.solutionOne.isUpper);
    ASSERT_EQ(false, lr.solutionTwo.isUpper);
}

TEST(LinearRestrictionTest, InvalidRestriction) {
    LinearRestriction lr = LinearRestriction({0,0,0,0}, {8,-1,0,0}, {16/3,-1,0,0});
    ASSERT_EQ(true, lr.alwaysFalse());
}