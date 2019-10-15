#ifndef HPNMG_LINEARBOUNDS_H
#define HPNMG_LINEARBOUNDS_H

#include "LinearDomain.h"
#include "LinearEquation.h"
#include "LinearRestriction.h"

#include <utility>
#include <vector>

namespace hpnmg {

    class LinearBoundsTree {
    public:
        class Node {
        private:
            bool decidable;
            bool root;
        public:
            bool isDecidable();
            bool isRoot();
            bool leaf;
            bool domainWasSet;

            LinearEquation equationOne;
            LinearEquation equationTwo;
            Node(LinearEquation equation);
            Node(LinearRestriction restriction);
            Node(const Node &node);

            int childLeftTrueId;
            int childLeftFalseId;
            int childRightTrueId;
            int childRightFalseId;
            int parentId;

            std::vector<LinearDomain> linearDomains;
        };
    public:

        std::vector<LinearBoundsTree::Node> nodes;
        LinearBoundsTree::Node getRootNode();
        std::vector<LinearBoundsTree::Node> getLeaves();

        std::vector<LinearDomain> getDomains();
        std::vector<LinearDomain> getUniqueDomains();

        LinearBoundsTree(Domain domain, LinearEquation newCondition);

        static Domain Repair(Domain domain);
    private:
        Domain domain;
        int create(Domain domain, LinearRestriction restriction, int parentId);

        bool shouldSetBound(LinearEquation eq, LinearBound bound);

        void setNode(int id, hpnmg::LinearBoundsTree::Node node);

        bool isValidBound(LinearEquation eq, LinearBound bound);

        std::pair<LinearRestriction, LinearRestriction> createNewRestrictions(LinearBound bound, LinearEquation equation);
    };
}

#endif //HPNMG_LINEARBOUNDS_H
