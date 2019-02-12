#include <gtest/gtest.h>
#include "helper/LinearEquation.h"

using namespace hpnmg;

TEST(LinearEquation, SimpleRightBound) {
    LinearEquation l = LinearEquation({0,0}, {3,-1});
    ASSERT_EQ(l.dependencyIndex, 0);
    ASSERT_EQ(l.equation[0], 3);
    ASSERT_EQ(l.isUpper, true);
    ASSERT_EQ(l.isDecidable(), true);
}

TEST(LinearEquation, SimpleLeftBound) {
    LinearEquation l = LinearEquation({4,0}, {3,1});
    ASSERT_EQ(l.dependencyIndex, 0);
    ASSERT_EQ(l.equation[0], 1);
    ASSERT_EQ(l.isUpper, false);
    ASSERT_EQ(l.isDecidable(), true);
}

TEST(LinearEquation, Undecidable) {
    LinearEquation l = LinearEquation({0,0,0}, {3,-1,-1});
    ASSERT_EQ(l.dependencyIndex, 1);
    ASSERT_EQ(l.equation[0], 3);
    ASSERT_EQ(l.equation[1], -1);
    ASSERT_EQ(l.isUpper, true);
    ASSERT_EQ(l.isDecidable(), false);
}

TEST(LinearEquation, AlwaysTrue) {
    LinearEquation l = LinearEquation({0,0,0}, {3,0,0});
    ASSERT_EQ(l.dependencyIndex, -1);
    ASSERT_EQ(l.alwaysTrue, true);
}

TEST(LinearEquation, NoFixValue) {
    LinearEquation l = LinearEquation({0,0,0}, {0,-3,0});
    ASSERT_EQ(l.dependencyIndex, 0);
    ASSERT_EQ(true, l.isUpper);
}

TEST(LinearEquation, FourDimension) {
    LinearEquation l({0,0,0,0,0}, {-3,1,1,1,1});
    ASSERT_EQ(l.dependencyIndex, 3);
    ASSERT_EQ(false, l.isUpper);
}

TEST(LinearEquation, AlwaysTrueNoDep) {
    LinearEquation l({0,0,0}, {10,0,0});
    ASSERT_EQ(true, l.alwaysTrue);
}

TEST(LinearEquation, OneDep) {
    LinearEquation equation({8,0,0,0}, {0,0,0,1});
    ASSERT_EQ(2, equation.dependencyIndex);
    ASSERT_EQ(false, equation.isUpper);
    ASSERT_EQ(8,equation.equation[0]);
}

