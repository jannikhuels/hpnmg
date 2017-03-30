#pragma once

#include "datastructures/ParametricLocation.h"
#include <map>
#include "exceptions/ParametricLocationTreeExceptions.h"
#include "STDiagram.h"

#define ROOT_NODE_INDEX 0
#define ROOT_NODE_ID 1

using namespace std;

namespace hpnmg {
    typedef int NODE_ID;
    typedef NODE_ID PARENT_NODE_ID;

    typedef std::pair<NODE_ID, ParametricLocation> ParametricLocationTreeNode;

    class ParametricLocationTree 
    {
    private:
        NODE_ID currentId;

        std::multimap<PARENT_NODE_ID, ParametricLocationTreeNode> parametricLocations;

        void insertParametricLocation(PARENT_NODE_ID parentNodeID, const ParametricLocation &parametricLocation);

        std::vector<ParametricLocationTreeNode> getNodes(NODE_ID id);

        void setRootNode(const ParametricLocation &rootLocation);

        bool isValidRootLocation(const ParametricLocation &rootLocation, string &reason);

        int maxTime;

    public:
        ParametricLocationTree(const ParametricLocation &rootLocation, int maxTime);

        ParametricLocationTreeNode getRootNode();

        std::vector<ParametricLocationTreeNode> getChildNodes(const ParametricLocationTreeNode parentNode);

        void setChildNode(const ParametricLocationTreeNode parentNode, const ParametricLocation &childLocation);

        int getDimension();

    }; 
}