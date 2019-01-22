#include <gtest/gtest.h>
#include "hpnmg/helper/LinearDomain.h"

using namespace hpnmg;

TEST(LinearDomain, Construction) {
    LinearEquation eq = LinearEquation({10,0}, true, 0);
    LinearDomain dom = LinearDomain(eq, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), true);
    ASSERT_EQ(dom.getDomain().size(), 1);
    ASSERT_EQ(dom.getDomain()[0].first.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].second.size(), 2);

    eq = LinearEquation({-10,0}, false, 0);
    dom = LinearDomain(eq, {{{0,0},{20,0}},{{0,0},{20,0}}});
    ASSERT_EQ(dom.isValid(), true);
}

TEST(LinearDomain, ConstructionTwo) {
    LinearEquation eq1 = LinearEquation({10,0}, true, 0);
    LinearEquation eq2 = LinearEquation({3,0}, false, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), true);
    ASSERT_EQ(dom.getDomain().size(), 1);
    ASSERT_EQ(dom.getDomain()[0].first.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].second.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].first[0], 3);
    ASSERT_EQ(dom.getDomain()[0].second[0], 10);
}

TEST(LinearDomain, ConstructionTwoWithLowerOneTrue) {
    LinearEquation eq1 = LinearEquation({10,0}, true, 0);
    LinearEquation eq2 = LinearEquation({-3,0}, false, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), true);
    ASSERT_EQ(dom.getDomain().size(), 1);
    ASSERT_EQ(dom.getDomain()[0].first.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].second.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].first[0], 0);
    ASSERT_EQ(dom.getDomain()[0].second[0], 10);
}

TEST(LinearDomain, ConstructionTwoWithUpperOneTrue) {
    LinearEquation eq1 = LinearEquation({21,0}, true, 0);
    LinearEquation eq2 = LinearEquation({3,0}, false, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), true);
    ASSERT_EQ(dom.getDomain().size(), 1);
    ASSERT_EQ(dom.getDomain()[0].first.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].second.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].first[0], 3);
    ASSERT_EQ(dom.getDomain()[0].second[0], 20);
}

TEST(LinearDomain, ConstructionTwoWithBothTrue) {
    LinearEquation eq1 = LinearEquation({21,0}, true, 0);
    LinearEquation eq2 = LinearEquation({-3,0}, false, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), true);
    ASSERT_EQ(dom.getDomain().size(), 1);
    ASSERT_EQ(dom.getDomain()[0].first.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].second.size(), 2);
    ASSERT_EQ(dom.getDomain()[0].first[0], 0);
    ASSERT_EQ(dom.getDomain()[0].second[0], 20);
}

TEST(LinearDomain, InvalidConstruction) {
    LinearEquation eq = LinearEquation({-10,0}, true, 0);
    LinearDomain dom = LinearDomain(eq, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), false);

    eq = LinearEquation({21,0}, false, 0);
    dom = LinearDomain(eq, {{{0,0},{20,0}},{{0,0},{20,0}}});

    ASSERT_EQ(dom.isValid(), false);
}

TEST(LinearDomain, AddEquation) {
    LinearEquation eq = LinearEquation({3,0,0}, true, 0);
    LinearDomain dom = LinearDomain(eq, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});

    LinearEquation eq2 = LinearEquation({3,-1,0}, true, 1);
    dom.addEquation(eq2);

    ASSERT_EQ(dom.getDomain().size(), 2);
    ASSERT_EQ(dom.getDomain()[1].first.size(), 0);
    ASSERT_EQ(dom.getDomain()[1].second.size(), 3);
}


TEST(LinearDomain, Intersection) {
    LinearEquation eq1 = LinearEquation({3,0,0}, true, 0);
    LinearDomain dom1 = LinearDomain(eq1, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq2 = LinearEquation({3,-1,0}, true, 1);
    dom1.addEquation(eq2);

    LinearEquation eq3 = LinearEquation({0.75,0,0}, true, 0);
    LinearDomain dom2 = LinearDomain(eq3, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq4 = LinearEquation({1.5,-1,0}, false, 1);
    dom2.addEquation(eq4);

    bool valid = dom1.intersect(dom2);

    ASSERT_EQ(valid, true);
    ASSERT_EQ(dom1.getDomain().size(), 2);
    ASSERT_EQ(dom1.getDomain()[0].first[0], 0);
    ASSERT_EQ(dom1.getDomain()[0].second[0], 0.75);
    ASSERT_EQ(dom1.getDomain()[1].first[0], 1.5);
    ASSERT_EQ(dom1.getDomain()[1].second[0], 3);
}

TEST(LinearDomain, FalseIntersection) {
    LinearEquation eq1 = LinearEquation({3,0,0}, true, 0);
    LinearDomain dom1 = LinearDomain(eq1, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq2 = LinearEquation({3,-1,0}, true, 1);
    dom1.addEquation(eq2);

    LinearEquation eq3 = LinearEquation({4,0,0}, true, 0);
    LinearDomain dom2 = LinearDomain(eq3, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq4 = LinearEquation({1.5,-1,0}, false, 1);
    dom2.addEquation(eq4);

    bool valid = dom1.intersect(dom2);

    ASSERT_EQ(valid, false);
}

TEST(LinearDomain, EmptyIntersection) {
    LinearEquation eq1 = LinearEquation({3,0,0}, true, 0);
    LinearDomain dom1 = LinearDomain(eq1, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq2 = LinearEquation({3,-1,0}, true, 1);
    dom1.addEquation(eq2);

    LinearEquation eq3 = LinearEquation({4,0,0}, false, 0);
    LinearDomain dom2 = LinearDomain(eq3, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq4 = LinearEquation({1.5,-1,0}, false, 1);
    dom2.addEquation(eq4);

    bool valid = dom1.intersect(dom2);

    ASSERT_EQ(valid, false);
}

TEST(LinearDomain, BoundIntersection) {
    LinearEquation eq1 = LinearEquation({4,0,0}, true, 0);
    LinearEquation eq2 = LinearEquation({0,0,0}, false, 0);
    LinearDomain dom1 = LinearDomain(eq1, eq2, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq5 = LinearEquation({3,-1,0}, true, 1);
    dom1.addEquation(eq5);

    LinearEquation eq3 = LinearEquation({5,0,0}, true, 0);
    LinearEquation eq4 = LinearEquation({1,0,0}, false, 0);
    LinearDomain dom2 = LinearDomain(eq3, eq4, {{{0,0,0},{20,0,0}},{{0,0,0},{20,0,0}}});
    LinearEquation eq6 = LinearEquation({5,-1,0}, true, 1);
    dom2.addEquation(eq6);

    bool valid = dom1.intersect(dom2);

    ASSERT_EQ(true, valid);
    ASSERT_EQ(1, dom1.getDomain()[0].first[0]);
    ASSERT_EQ(4, dom1.getDomain()[0].second[0]);
    ASSERT_EQ(5, dom1.getDomain()[1].second[0]);
}

TEST(LinearDomain, ByRestriction) {
    LinearEquation eq1 = LinearEquation({0,0,0}, true, 0);
    LinearEquation eq2 = LinearEquation({-3,0,0}, false, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0,0},{10,0,0}},{{0,0,0},{10,0,0}},{{0,0,0},{10,0,0}}});
    ASSERT_EQ(false, dom.isValid());
}

TEST(LinearDomain, ByRestrictionTwo) {
    LinearEquation eq1 = LinearEquation({0,0,0}, false, 0);
    LinearEquation eq2 = LinearEquation({-3,0,0}, true, 0);
    LinearDomain dom = LinearDomain(eq1, eq2, {{{0,0,0},{10,0,0}},{{0,0,0},{10,0,0}},{{0,0,0},{10,0,0}}});
    ASSERT_EQ(false, dom.isValid());
}