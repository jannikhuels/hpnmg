#include "ParametricLocationTree.h"

namespace hpnmg {

    ParametricLocationTree::ParametricLocationTree(const ParametricLocation &rootLocation, int maxTime) : currentId(ROOT_NODE_ID), maxTime(maxTime) {
        setRootNode(rootLocation);
    }

    ParametricLocationTreeNode ParametricLocationTree::getRootNode() {
        return getNodes(ROOT_NODE_INDEX)[0];
    }

    std::vector<ParametricLocationTreeNode> ParametricLocationTree::getChildNodes(const ParametricLocationTreeNode parentNode) {
        if (parentNode.first >= ROOT_NODE_ID) {
            return getNodes(parentNode.first);
        } 
        throw InvalidParentNodeException();
    }

    void ParametricLocationTree::setChildNode(const ParametricLocationTreeNode parentNode, const ParametricLocation &childLocation) {
        insertParametricLocation(parentNode.first, childLocation);
    }

    std::vector<ParametricLocationTreeNode> ParametricLocationTree::getNodes(PARENT_NODE_ID id) {
        typedef std::multimap<PARENT_NODE_ID, ParametricLocationTreeNode>::iterator mmiterator;
        std::vector<ParametricLocationTreeNode> nodes;

        std::pair <mmiterator, mmiterator> nodeRange;
        nodeRange = parametricLocations.equal_range(id);

        for (mmiterator iterator = nodeRange.first; iterator != nodeRange.second; iterator++) {
            nodes.push_back(iterator->second);
        }

        if (nodes.size() > 0) {
            return nodes;
        }
        throw NoSuchNodeException(id);
    }

    int ParametricLocationTree::getDimension() {
        return getRootNode().second.getDimension();
    }

    void ParametricLocationTree::setRootNode(const ParametricLocation &rootLocation){
        string errorReason;
        if(!isValidRootLocation(rootLocation, errorReason)) {
            throw IllegalArgumentException("rootLocation", errorReason);
        }
        if (parametricLocations.size() == 0 && currentId == ROOT_NODE_ID) {
            insertParametricLocation(ROOT_NODE_INDEX, rootLocation);
        } else {
            throw RootNodeExistsException();
        }
    }

    bool ParametricLocationTree::isValidRootLocation(const ParametricLocation &rootLocation, string &reason) {
        if (rootLocation.getSourceEvent().getEventType() != EventType::Root) {
            reason = "Invalid Root Node EventType.";
            return false;
        }
        return true;
    }

    void ParametricLocationTree::insertParametricLocation(PARENT_NODE_ID parentNodeId, const ParametricLocation &parametricLocation) {
        if (parentNodeId >= ROOT_NODE_INDEX && parentNodeId < currentId) {
            ParametricLocationTreeNode newNode(currentId, parametricLocation);
            parametricLocations.insert(std::pair<int, ParametricLocationTreeNode>(parentNodeId,newNode));
            currentId++;
        } else {
            throw InvalidParentNodeException();
        }
    }

}
