#include <gtest/gtest.h>
#include "helper/LinearBoundsTree.h"

using namespace hpnmg;

TEST(LinearBoundsTree, SimpleTree) {
    LinearEquation equation({3,0}, true, 0);
    LinearBoundsTree tree = LinearBoundsTree({{{0,0},{10,0}}, {{0,0},{10,20}}}, equation);
    LinearBoundsTree::Node root = tree.nodes[0];

    ASSERT_EQ(-1, root.childLeftTrueId);
    ASSERT_EQ(tree.nodes.size(),1);
}

TEST(LinearBoundsTree, TwoDependencyTree) {
    LinearEquation equation({0,0,0},{-3,1,1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0,0},{10,0,0}}, {{0,0,0},{10,0,0}}}, equation);
    ASSERT_EQ(tree.nodes.size(),3);

    LinearBoundsTree::Node rootNode = tree.getRootNode();
    ASSERT_EQ(1, rootNode.childLeftTrueId);
    ASSERT_EQ(2, rootNode.childLeftFalseId);

    LinearEquation leftMost = tree.nodes[rootNode.childLeftTrueId].equationOne;

    ASSERT_EQ(leftMost.equation[0], 3);

    std::vector<LinearBoundsTree::Node> leaves = tree.getLeaves();
    ASSERT_EQ(leaves.size(), 2);
}

TEST(LinearBoundsTree, ThreeDependencyTree) {
    LinearEquation equation({0,0,0,0}, {3,-1,-1,-1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{0,3,1,0}}}, equation);
    ASSERT_EQ(tree.nodes.size(),11);

    std::vector<LinearBoundsTree::Node> leaves = tree.getLeaves();
    ASSERT_EQ(8, leaves.size());
    ASSERT_EQ(true, leaves[0].equationOne.isUpper);
    ASSERT_EQ(3, leaves[0].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[0].equationOne.dependencyIndex);
    ASSERT_EQ(false, leaves[0].equationTwo.isUpper);
    ASSERT_EQ(-7, leaves[0].equationTwo.equation[0]);
    ASSERT_EQ(0, leaves[0].equationTwo.dependencyIndex);

    ASSERT_EQ(true, leaves[1].equationOne.alwaysTrue);
    ASSERT_EQ(true, leaves[1].equationTwo.isUpper);
    ASSERT_EQ(-7, leaves[1].equationTwo.equation[0]);
    ASSERT_EQ(0, leaves[1].equationTwo.dependencyIndex);

    ASSERT_EQ(true, leaves[2].equationOne.isUpper);
    ASSERT_EQ(0.75, leaves[2].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[2].equationOne.dependencyIndex);
    ASSERT_EQ(false, leaves[2].equationTwo.isUpper);
    ASSERT_EQ(-4.25, leaves[2].equationTwo.equation[0]);
    ASSERT_EQ(0, leaves[2].equationTwo.dependencyIndex);

    ASSERT_EQ(false, leaves[3].equationOne.isUpper);
    ASSERT_EQ(0.75, leaves[3].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[3].equationOne.dependencyIndex);
    ASSERT_EQ(true, leaves[3].equationTwo.alwaysTrue);

    ASSERT_EQ(true, leaves[4].equationOne.isUpper);
    ASSERT_EQ(0, leaves[4].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[4].equationOne.dependencyIndex);
    ASSERT_EQ(false, leaves[4].equationTwo.isUpper);
    ASSERT_EQ(0, leaves[4].equationTwo.dependencyIndex);

    ASSERT_EQ(false, leaves[5].equationOne.isUpper);
    ASSERT_EQ(0, leaves[5].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[5].equationOne.dependencyIndex);
    ASSERT_EQ(true, leaves[5].equationTwo.alwaysTrue);

    ASSERT_EQ(true, leaves[6].equationOne.isUpper);
    ASSERT_EQ(0.75, leaves[6].equationOne.equation[0]);
    ASSERT_EQ(0, leaves[6].equationOne.dependencyIndex);
    ASSERT_EQ(false, leaves[6].equationTwo.isUpper);
    ASSERT_EQ(-4.25, leaves[6].equationTwo.equation[0]);
    ASSERT_EQ(0, leaves[6].equationTwo.dependencyIndex);

    ASSERT_EQ(true, leaves[7].equationOne.alwaysTrue);
    ASSERT_EQ(true, leaves[7].equationTwo.isUpper);
    ASSERT_EQ(-4.25, leaves[7].equationTwo.equation[0]);
    ASSERT_EQ(0, leaves[7].equationTwo.dependencyIndex);
}

TEST(LinearBoundsTree, DomainTest) {
    LinearEquation equation({0,0,0},{-3,1,1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0,0},{10,0,0}}, {{0,0,0},{10,0,0}}}, equation);

    std::vector<LinearBoundsTree::Node> leaves = tree.getLeaves();
    ASSERT_EQ(2, leaves.size());

    ASSERT_EQ(1, leaves[0].linearDomains.size());
    LinearDomain lin = leaves[0].linearDomains[0];
    ASSERT_EQ(0, lin.getDomain()[0].first[0]);
    ASSERT_EQ(3, lin.getDomain()[0].second[0]);

    ASSERT_EQ(1, leaves[1].linearDomains.size());
    lin = leaves[1].linearDomains[0];
    ASSERT_EQ(3, lin.getDomain()[0].first[0]);
    ASSERT_EQ(10, lin.getDomain()[0].second[0]);

    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(2, domains.size());
    ASSERT_EQ(2, domains[0].getDomain().size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0},{3,0,0}}, {{3,-1,0},{10,0,0}}}));
    ASSERT_EQ(true, domains[1].equals({{{3,0,0},{10,0,0}}, {{0,0,0},{10,0,0}}}));
}

TEST(LinearBoundsTree, ThreeDomainTest) {
    LinearEquation equation({0,0,0,0}, {3,-1,-1,-1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{0,3,1,0}}}, equation);

    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(3, domains.size());

    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0},{0.75,0,0,0}}, {{1.5,-2,0,0},{3,-1,0,0}}, {{0,0,0,0},{3,-1,-1,0}}}));
    ASSERT_EQ(true, domains[1].equals({{{0.75,0,0,0},{3,0,0,0}}, {{0,0,0,0},{3,-1,0,0}}, {{0,0,0,0},{3,-1,-1,0}}}));
    ASSERT_EQ(true, domains[2].equals({{{0,0,0,0},{0.75,0,0,0}}, {{0,0,0,0},{1.5,-2,0,0}}, {{0,0,0,0},{0,3,1,0}}}));
}

TEST(LinearBoundsTree, FourDomainTest) {
    LinearEquation equation({0,0,0,0,0}, {-3,1,1,1,1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}}, equation);

    std::vector<LinearBoundsTree::Node> leaves = tree.getLeaves();

    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(4, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0,0},{3,0,0,0,0}}, {{0,0,0,0,0},{3,-1,0,0,0}}, {{0,0,0,0,0},{3,-1,-1,0,0}}, {{3,-1,-1,-1,0},{10,0,0,0,0}}}));
    ASSERT_EQ(true, domains[1].equals({{{0,0,0,0,0},{3,0,0,0,0}}, {{0,0,0,0,0},{3,-1,0,0,0}}, {{3,-1,-1,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}}));
    ASSERT_EQ(true, domains[2].equals({{{0,0,0,0,0},{3,0,0,0,0}}, {{3,-1,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}}));
    ASSERT_EQ(true, domains[3].equals({{{3,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}, {{0,0,0,0,0},{10,0,0,0,0}}}));
}

TEST(LinearBoundsTree, SingleDomainTest) {
    LinearEquation equation({0,0}, {3,-1});
    LinearBoundsTree tree = LinearBoundsTree({{{0,0}, {10,0}}}, equation);

    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0}, {3,0}}}));
}

TEST(LinearBoundsTree, Error5Dimensions) {
    LinearEquation equation({6,-4,-3,-2,0}, false, 3);
    LinearBoundsTree tree({{{0,0,0,0,0},{3,0,0,0,0}}, {{0,0,0,0,0},{3,-1,0,0,0}}, {{0,0,0,0,0},{3,-1,-1,0,0}}, {{0,0,0,0,0},{3,-1,-1,-1,0}}}, equation);
    std::vector<LinearDomain> dms = tree.getDomains();
}

TEST(LinearBoundsTree, ErrorThreeParallel) {
    LinearEquation equation({0,0,0,0}, {4,-1,-1,0});
    LinearBoundsTree tree({{{0,0,0,0},{10,0,0,0}}, {{0,1,0,0},{10,0,0,0}}, {{0,1,1,0},{10,0,0,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0},{2,0,0,0}}, {{0,1,0,0},{4,-1,0,0}}, {{0,1,1,0},{10,0,0,0}}}));

    LinearEquation equation2({8,0,0,0}, {0,0,0,1});
    LinearBoundsTree tree2(domains[0].getDomain(), equation2);
    std::vector<LinearDomain> domains2 = tree2.getDomains();
    ASSERT_EQ(1, domains2.size());
    ASSERT_EQ(true, domains2[0].equals({{{0,0,0,0},{2,0,0,0}}, {{0,1,0,0},{4,-1,0,0}}, {{8,0,0,0},{10,0,0,0}}}));
}

TEST(LinearBoundsTree, ErrorThreeParallel2) {
    LinearEquation equation({0,0,0,0}, {8,-1,-1,0});
    LinearBoundsTree tree({{{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{0,3,1,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0},{8,0,0,0}}, {{0,0,0,0},{8,-1,0,0}}, {{0,0,0,0},{0,3,1,0}}}));

    LinearEquation equation2({8,0,0,0}, {0,1,1,1});
    LinearBoundsTree tree2(domains[0].getDomain(), equation2);

    std::vector<LinearBoundsTree::Node> leaves = tree2.getLeaves();

    std::vector<LinearDomain> domains2 = tree2.getUniqueDomains();
    ASSERT_EQ(2, domains2.size());
    ASSERT_EQ(true, domains2[0].equals({{{0,0,0,0},{2,0,0,0}}, {{4,-2,0,0},{8,-1,0,0}}, {{8,-1,-1,0},{0,3,1,0}}}));
    ASSERT_EQ(true, domains2[1].equals({{{2,0,0,0},{8,0,0,0}}, {{0,0,0,0},{8,-1,0,0}}, {{8,-1,-1,0},{0,3,1,0}}}));
}

TEST(LinearBoundsTree, ErrorThreeParallel3) {
    LinearEquation equation({0,0,0,0}, {8,0,0,-1});
    LinearBoundsTree tree({{{0,0,0,0},{10,0,0,0}}, {{0,1,0,0},{10,0,0,0}}, {{0,1,1,0},{0,2,2,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();
    ASSERT_EQ(3, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0},{2,0,0,0}}, {{4,-1,0,0},{8,-1,0,0}}, {{0,1,1,0},{8,0,0,0}}}));
    ASSERT_EQ(true, domains[1].equals({{{2,0,0,0},{4,0,0,0}}, {{0,1,0,0},{8,-1,0,0}}, {{0,1,1,0},{8,0,0,0}}}));
    ASSERT_EQ(true, domains[2].equals({{{0,0,0,0},{2,0,0,0}}, {{0,1,0,0},{4,-1,0,0}}, {{0,1,1,0},{0,2,2,0}}}));

    LinearEquation equation2({8,0,0,0}, {0,1,1,0.5});

    LinearBoundsTree tree2(domains[0].getDomain(), equation2);
    std::vector<LinearDomain> domains2 = tree2.getUniqueDomains();
    ASSERT_EQ(2, domains2.size());

    LinearBoundsTree tree3(domains[1].getDomain(), equation2);
    std::vector<LinearDomain> domains3 = tree3.getUniqueDomains();
    ASSERT_EQ(3, domains3.size());

    LinearBoundsTree tree4(domains[2].getDomain(), equation2);
    std::vector<LinearDomain> domains4 = tree4.getUniqueDomains();
    ASSERT_EQ(0, domains4.size());
}

TEST(LinearBoundsTree, ErrorFour) {
    LinearEquation equation({0,1,1,1,0}, {8,0,0,0,0});
    LinearBoundsTree tree({{{0,0,0,0,0},{10,0,0,0,0}}, {{0,1,0,0,0},{10,0,0,0,0}}, {{0,1,1,0,0},{10,0,0,0,0}},{{0,1,1,1,0},{10,0,0,0,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0,0},{2,0,0,0,0}}, {{0,1,0,0,0},{4,-1,0,0,0}}, {{0,1,1,0,0},{8,-1,-1,0,0}},{{0,1,1,1,0},{10,0,0,0,0}}}));

    LinearEquation equation2({8,0,0,0,0}, {0,0,0,0,1});

    LinearBoundsTree tree2(domains[0].getDomain(), equation2);

    std::vector<LinearDomain> domains2 = tree2.getUniqueDomains();
    ASSERT_EQ(1, domains2.size());
    ASSERT_EQ(true, domains2[0].equals({{{0,0,0,0,0},{2,0,0,0,0}}, {{0,1,0,0,0},{4,-1,0,0,0}}, {{0,1,1,0,0},{8,-1,-1,0,0}},{{8,0,0,0,0},{10,0,0,0,0}}}));
}

TEST(LinearBoundsTree, RepairResult)
{
    LinearEquation equation({0,2,1,0}, {0,0,0,1});
    LinearBoundsTree tree({{{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0,0},{5,0,0,0}}, {{0,0,0,0},{10,-2,0,0}}, {{0,2,1,0},{10,0,0,0}}}));
}

TEST(LinearBoundsTree, Repair)
{
    Domain d = LinearBoundsTree::Repair({{{0,0,0,0},{10,0,0,0}}, {{0,0,0,0},{10,0,0,0}}, {{0,2,1,0},{10,0,0,0}}});
    LinearDomain ld(d);
    ASSERT_EQ(true, ld.equals({{{0,0,0,0},{5,0,0,0}}, {{0,0,0,0},{10,-2,0,0}}, {{0,2,1,0},{10,0,0,0}}}));
}

TEST(LinearBoundsTree, ErrorFiveOne) {
    LinearEquation equation({0, 1, 1, 1, 0}, {8, 0, 0, 0, 0});
    LinearBoundsTree tree({{{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                           {{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                           {{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                           {{0, 0, 0, 0, 0}, {0, 2, 1, 0, 0}}}, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();
    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0, 0, 0, 0, 0}, {8,  0,  0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {8,  -1, 0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {8,  -1, -1, 0, 0}},
                                       {{0, 0, 0, 0, 0}, {0, 2,  1,  0, 0}}}));

    LinearEquation equation2({8, 0, 0, 0, 0}, {0, 1, 1, 1, 1});

    LinearBoundsTree tree2(domains[0].getDomain(), equation2);
    std::vector<LinearDomain> domains2 = tree2.getUniqueDomains();
    ASSERT_EQ(3, domains2.size());
    /*ASSERT_EQ(true, domains2[0].equals({{{0, 0,  0,  0,  0}, {8,  0,  0,  0, 0}},
                                        {{0, 0,  0,  0,  0}, {8,  -1, 0,  0, 0}},
                                        {{0, 0,  0,  0,  0}, {8,  -1, -1, 0, 0}},
                                        {{8, -1, -1, -1, 0}, {10, 0,  0,  0, 0}}}));*/
}

TEST(LinearBoundsTree, ErrorFiveTwo) {
    LinearEquation equation({0, 0, 0, 0, 0}, {8, -1, -1, -1, 0});
    Domain d = LinearBoundsTree::Repair({{{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                                         {{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                                         {{0, 0, 0, 0, 0}, {10, 0, 0, 0, 0}},
                                         {{0, 2, 1, 0, 0}, {10, 0, 0, 0, 0}}});
    LinearBoundsTree tree(d, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();
    ASSERT_EQ(2, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0, 0, 0, 0, 0}, {2,  0,  0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {8,  -1, 0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {8,  -1, -1, 0, 0}},
                                       {{0, 2, 1, 0, 0}, {10, 0,  0,  0, 0}}}));
    ASSERT_EQ(true, domains[1].equals({{{2, 0, 0, 0, 0}, {5,  0,  0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {10, -2, 0,  0, 0}},
                                       {{0, 0, 0, 0, 0}, {8,  -1, -1, 0, 0}},
                                       {{0, 2, 1, 0, 0}, {10, 0,  0,  0, 0}}}));

    LinearEquation equation2({8, 0, 0, 0, 0}, {0, 3, 2, 1, 0});

    LinearBoundsTree tree2(domains[0].getDomain(), equation2);
    std::vector<LinearDomain> domains2 = tree2.getUniqueDomains();
    ASSERT_EQ(2, domains2.size());

    LinearBoundsTree tree3(domains[1].getDomain(), equation2);
    std::vector<LinearDomain> domains3 = tree3.getUniqueDomains();
    ASSERT_EQ(3, domains3.size());
}

TEST(LinearBoundsTree, Carina){

    LinearEquation equation({0,0,1}, {0.5,0,0}); //<=
    LinearBoundsTree tree({{{0,0,0}, {2,0,0}}, {{0,1,0}, {6,-2,0}}}, equation);
    std::vector<LinearDomain> domains = tree.getUniqueDomains();

    ASSERT_EQ(1, domains.size());
    ASSERT_EQ(true, domains[0].equals({{{0,0,0}, {0.5,0,0}}, {{0,1,0}, {0.5,0,0}}}));
    //ASSERT_EQ(true, domains[1].equals({{{0.5,0,0}, {2,0,0}}, {{0,1,0}, {6,-2,0}}}));

}