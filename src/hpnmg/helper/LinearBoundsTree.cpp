#include "LinearBoundsTree.h"

namespace hpnmg {

    LinearBoundsTree::Node::Node(LinearEquation equation) {
        this->equationOne = equation;
        this->equationTwo = LinearEquation();
        this->root = true;
        this->decidable = equation.isDecidable();
        this->leaf = false;
        this->domainWasSet = false;
    }

    LinearBoundsTree::Node::Node(hpnmg::LinearRestriction restriction) {
        this->equationOne = restriction.solutionOne;
        this->equationTwo = restriction.solutionTwo;
        this->root = false;
        this->decidable = restriction.solutionOne.isDecidable() && restriction.solutionTwo.isDecidable();
        this->leaf = false;
        this->domainWasSet = false;
    }

    LinearBoundsTree::Node::Node(const LinearBoundsTree::Node &node){
        this->equationOne = node.equationOne;
        this->equationTwo = node.equationTwo;
        this->root = node.root;
        this->decidable = node.decidable;
        this->childLeftTrueId = node.childLeftTrueId;
        this->childLeftFalseId = node.childLeftFalseId;
        this->childRightTrueId = node.childRightTrueId;
        this->childRightFalseId = node.childRightFalseId;
        this->parentId = node.parentId;
        this->linearDomains = node.linearDomains;
        this->leaf = node.leaf;
        this->domainWasSet = node.domainWasSet;
    }

    bool LinearBoundsTree::Node::isDecidable() {
        return this->decidable;
    }

    bool LinearBoundsTree::Node::isRoot() {
        return this->root;
    }

    LinearBoundsTree::LinearBoundsTree(Domain domain, LinearEquation newCondition) {
        int depIndex = newCondition.dependencyIndex;

        assert(domain.size() > depIndex);

        std::pair<LinearRestriction, LinearRestriction> newRestrictions = this->createNewRestrictions(domain[depIndex], newCondition);

        LinearRestriction restrictionValidCondition = newRestrictions.first;
        LinearRestriction restrictionInvalidCondition = newRestrictions.second;

        Node rootNode = Node(newCondition);
        this->nodes.push_back(rootNode);

        if (!newCondition.isDecidable()) {
            rootNode.childLeftTrueId = this->create(domain, restrictionValidCondition, 0);
            rootNode.childLeftFalseId = this->create(domain, restrictionInvalidCondition, 0);
            rootNode.childRightTrueId = -1;
            rootNode.childRightFalseId = -1;
        } else {
            LinearDomain lin = LinearDomain(newCondition, domain);
            if (lin.isValid()) {
                lin.fill(domain);
                rootNode.linearDomains.push_back(lin);
            }
            rootNode.childLeftTrueId = -1;
            rootNode.childLeftFalseId = -1;
            rootNode.childRightTrueId = -1;
            rootNode.childRightFalseId = -1;
        }

        this->nodes[0] = rootNode;
        this->domain = domain;
    }

    int LinearBoundsTree::create(Domain domain, LinearRestriction newCondition, int parentId) {

        LinearBoundsTree::Node currentNode(newCondition);
        currentNode.parentId = parentId;
        int nodeId = this->nodes.size();
        this->nodes.push_back(currentNode);


        if (!newCondition.solutionOne.isDecidable()) {
            std::pair<LinearRestriction, LinearRestriction> newRestrictionsValid = this->createNewRestrictions(domain[newCondition.solutionOne.dependencyIndex], newCondition.solutionOne);
            currentNode.childLeftTrueId = this->create(domain, newRestrictionsValid.first, nodeId);
            currentNode.childLeftFalseId = this->create(domain, newRestrictionsValid.second, nodeId);
        } else {
            currentNode.childLeftTrueId = -1;
            currentNode.childLeftFalseId = -1;
        }

        if (!newCondition.solutionTwo.isDecidable()) {
            std::pair<LinearRestriction, LinearRestriction> newRestrictionsValid = this->createNewRestrictions(domain[newCondition.solutionTwo.dependencyIndex], newCondition.solutionTwo);
            currentNode.childRightTrueId = this->create(domain, newRestrictionsValid.first, nodeId);
            currentNode.childRightFalseId = this->create(domain, newRestrictionsValid.second, nodeId);
        } else {
            currentNode.childRightTrueId = -1;
            currentNode.childRightFalseId = -1;
        }

        if (newCondition.solutionOne.isDecidable() && newCondition.solutionTwo.isDecidable()) {
            if (!newCondition.solutionOne.alwaysTrue || !newCondition.solutionTwo.alwaysTrue) {
                currentNode.leaf = true;
            }
            LinearDomain lin = LinearDomain(newCondition.solutionOne, newCondition.solutionTwo, domain);
            if (lin.isValid()) {
                currentNode.linearDomains.push_back(lin);
            }
            currentNode.domainWasSet = true;
        } else if (newCondition.solutionOne.isDecidable() && !newCondition.solutionOne.alwaysTrue) {
            LinearDomain lin = LinearDomain(newCondition.solutionOne, domain);
            if (lin.isValid()) {
                currentNode.linearDomains.push_back(lin);
            }
            currentNode.domainWasSet = true;
        } else if (newCondition.solutionTwo.isDecidable() && !newCondition.solutionTwo.alwaysTrue) {
            LinearDomain lin = LinearDomain(newCondition.solutionTwo, domain);
            if (lin.isValid()) {
                currentNode.linearDomains.push_back(lin);
            }
            currentNode.domainWasSet = true;
        }

        this->nodes[nodeId] = currentNode;
        return nodeId;
    }

    LinearBoundsTree::Node LinearBoundsTree::getRootNode() {
        return this->nodes[0];
    }

    std::vector<LinearBoundsTree::Node> LinearBoundsTree::getLeaves() {
        std::vector<LinearBoundsTree::Node> leaves;
        for (LinearBoundsTree::Node n : this->nodes) {
            if (n.leaf) {
                leaves.push_back(n);
            }
        }
        return leaves;
    }

    std::vector<LinearDomain> combineDomains(std::vector<LinearDomain> aTrue, std::vector<LinearDomain> aFalse, std::vector<LinearDomain> bTrue, std::vector<LinearDomain> bFalse) {
        std::vector<LinearDomain> combination;
        for (LinearDomain l : aTrue) {
            if (l.isValid()) {
                for (LinearDomain f : bTrue) {
                    LinearDomain c = l;
                    if (f.isValid() && c.intersect(f)) {
                        combination.push_back(c);
                    }
                }
                for (LinearDomain f : bFalse) {
                    LinearDomain c = l;
                    if (f.isValid() && c.intersect(f)) {
                        combination.push_back(c);
                    }
                }
            }
        }

        for (LinearDomain l : aFalse) {
            if (l.isValid()) {
                for (LinearDomain f : bTrue) {
                    LinearDomain c = l;
                    if (f.isValid() && c.intersect(f)) {
                        combination.push_back(c);
                    }
                }
                for (LinearDomain f : bFalse) {
                    LinearDomain c = l;
                    if (f.isValid() && c.intersect(f)) {
                        combination.push_back(c);
                    }
                }
            }
        }
        return combination;
    }

    std::vector<LinearDomain> addEquation(std::vector<LinearDomain> lds, LinearEquation eq) {
        std::vector<LinearDomain> res;
        for(LinearDomain ld : lds) {
            ld.addEquation(eq);
            res.push_back(ld);
        }
        return res;
    }

    std::vector<LinearDomain> LinearBoundsTree::getDomains() {
        LinearBoundsTree::Node root = this->getRootNode();

        if(root.linearDomains.size() > 0) {
            return root.linearDomains;
        }

        std::vector<LinearBoundsTree::Node> nodesToVisit = this->getLeaves();

        for (int i = 0; i < nodesToVisit.size(); i++) {
            LinearBoundsTree::Node node = nodesToVisit[i];
            LinearBoundsTree::Node parent = this->nodes[node.parentId];
            if (!parent.domainWasSet) {
                if (!parent.isRoot()) {
                    if (!parent.equationOne.alwaysTrue && !parent.equationTwo.alwaysTrue) {
                        std::vector<LinearDomain> dLeftTrue = this->nodes[parent.childLeftTrueId].linearDomains;
                        dLeftTrue = addEquation(dLeftTrue, parent.equationOne);
                        std::vector<LinearDomain> dLeftFalse = this->nodes[parent.childLeftFalseId].linearDomains;

                        std::vector<LinearDomain> dRightTrue = this->nodes[parent.childRightTrueId].linearDomains;
                        dRightTrue = addEquation(dRightTrue, parent.equationTwo);
                        std::vector<LinearDomain> dRightFalse = this->nodes[parent.childRightFalseId].linearDomains;

                        parent.linearDomains = combineDomains(dLeftTrue, dLeftFalse, dRightTrue, dRightFalse);
                        parent.domainWasSet = true;
                        this->setNode(node.parentId, parent);

                    } else {
                        if (!parent.equationOne.alwaysTrue) {
                            std::vector<LinearDomain> dLeftTrue = this->nodes[parent.childLeftTrueId].linearDomains;
                            dLeftTrue = addEquation(dLeftTrue, parent.equationOne);
                            std::vector<LinearDomain> dLeftFalse = this->nodes[parent.childLeftFalseId].linearDomains;
                            parent.linearDomains.insert(parent.linearDomains.end(), dLeftTrue.begin(), dLeftTrue.end());
                            parent.linearDomains.insert(parent.linearDomains.end(), dLeftFalse.begin(), dLeftFalse.end());
                            parent.domainWasSet = true;
                            this->setNode(node.parentId, parent);
                        }
                        if (!parent.equationTwo.alwaysTrue) {
                            std::vector<LinearDomain> dRightTrue = this->nodes[parent.childRightTrueId].linearDomains;
                            dRightTrue = addEquation(dRightTrue, parent.equationTwo);
                            std::vector<LinearDomain> dRightFalse = this->nodes[parent.childRightFalseId].linearDomains;
                            parent.linearDomains.insert(parent.linearDomains.end(), dRightTrue.begin(), dRightTrue.end());
                            parent.linearDomains.insert(parent.linearDomains.end(), dRightFalse.begin(), dRightFalse.end());
                            parent.domainWasSet = true;
                            this->setNode(node.parentId, parent);
                        }
                    }
                    nodesToVisit.push_back(parent);
                } else {
                    LinearBoundsTree::Node left = this->nodes[parent.childLeftTrueId];
                    for(LinearDomain ld : left.linearDomains) {
                        ld.fill(this->domain);
                        if (!parent.equationOne.alwaysTrue) {
                            ld.setSingleEquation(parent.equationOne);
                            //ld.addEquation(parent.equationOne);
                        }
                        ld.fill(this->domain);
                        parent.linearDomains.push_back(ld);
                    }

                    LinearBoundsTree::Node right = this->nodes[parent.childLeftFalseId];
                    for(LinearDomain ld : right.linearDomains) {
                        ld.fill(this->domain);
                        parent.linearDomains.push_back(ld);
                    }
                    parent.domainWasSet = true;
                    this->setNode(0, parent);
                }
            }
        }

        return this->getRootNode().linearDomains;
    }

    bool LinearBoundsTree::shouldSetBound(LinearEquation eq, LinearBound bound) {
        double value = eq.equation[0];
        return value >= bound.first[0] && value <= bound.second[0];
    }

    bool LinearBoundsTree::isValidBound(hpnmg::LinearEquation eq, hpnmg::LinearBound bound) {
        double value = eq.equation[0];
        if (eq.isUpper) {
            return !(value >= bound.second[0]);
        } else {
            return !(value <= bound.first[0]);
        }
    }

    std::pair<LinearRestriction, LinearRestriction> LinearBoundsTree::createNewRestrictions(hpnmg::LinearBound bound,
                                                                                            hpnmg::LinearEquation equation) {
        LinearRestriction restrictionValidCondition = LinearRestriction(bound.first, equation.equation, bound.second);

        if (equation.isUpper) {
            return {restrictionValidCondition, LinearRestriction(bound.first, bound.second, equation.equation)};
        } else {
            return {restrictionValidCondition, LinearRestriction(equation.equation, bound.first, bound.second)};
        }
    }

    void LinearBoundsTree::setNode(int id, hpnmg::LinearBoundsTree::Node node) {
        this->nodes[id] = node;
    }

    bool containsDomain(std::vector<LinearDomain> domains, LinearDomain toCompare) {
        for (LinearDomain d: domains) {
            if (d.equals(toCompare.getDomain())) {
                return true;
            }
        }
        return false;
    }

    std::vector<LinearDomain> LinearBoundsTree::getUniqueDomains() {
        std::vector<LinearDomain> domains = this->getDomains();
        std::vector<LinearDomain> uniqueDomains;

        for (LinearDomain d : domains) {
            if (!containsDomain(uniqueDomains, d) && !d.isEmpty()) {
                uniqueDomains.push_back(d);
            }
        }

        return uniqueDomains;
    }

    Domain LinearBoundsTree::Repair(hpnmg::Domain domain) {
        bool restrictionFound = false;
        LinEq first;
        LinEq second;
        for (LinearBound b : domain) {
            if (Computation::getDependencyIndex(b.first) > 0 || Computation::getDependencyIndex(b.second) > 0) {
                restrictionFound = true;
                first = b.first;
                second = b.second;
            }
        }

        if (restrictionFound) {
            LinearEquation linEq(first, second);
            if (!linEq.alwaysTrue) {
                LinearBoundsTree tree(domain, linEq);
                std::vector<LinearDomain> uniqueDomain = tree.getUniqueDomains();
                if (uniqueDomain.size() > 0) {
                    return uniqueDomain[0].getDomain();
                } else {
                    return domain;
                }
            } else {
                return domain;
            }
        } else {
            return domain;
        }
    }
}