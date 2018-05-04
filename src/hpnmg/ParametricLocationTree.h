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

    class ParametricLocationTree 
    {
    public:
        class Node {
        private:
            NODE_ID id;
            Region region;
            ParametricLocation parametricLocation;

        public:
            Node(NODE_ID id, const ParametricLocation &parametricLocation);
            NODE_ID getNodeID() const;
            Region getRegion() const;
            void setRegion(const Region &region);
            ParametricLocation getParametricLocation() const;
        };
    private:
        NODE_ID currentId;

        std::multimap<PARENT_NODE_ID, Node> parametricLocations;

        Node insertParametricLocation(PARENT_NODE_ID parentNodeID, const ParametricLocation &parametricLocation);
       
        std::vector<Node> getNodes(PARENT_NODE_ID parentID);

        void setRootNode(const ParametricLocation &rootLocation);

        bool isValidRootLocation(const ParametricLocation &rootLocation, string &reason);

        int maxTime;

        int dimension;

        Region baseRegion;

        void recursivelySetRegions(Node &startNode, Region &baseRegion);

        void recursivelyCollectRegions(const Node &startNode, vector<Region> &regions);

        void recursivelyCollectCandidateLocations(const Node &startNode, vector<Node> &candidates, std::pair<bool, Region> (*isCandidate)(const std::pair<double,double> &interval, const Region &region, int dimension), std::pair<double, double> interval, int dimension);

        std::vector<Event> getSourceEventsFromNodes(const std::vector<Node> &nodes);

        void recursivelyPrintRegions(const ParametricLocationTree::Node &startNode, int depth);

        std::vector<int> getDimensionRecursively(const ParametricLocationTree::Node &startNode, int numberOfGeneralTransitions);

        void addNormedDependenciesRecursively(const ParametricLocationTree::Node &startNode, std::vector<int> genTransOccurings, int dimension);

    public:

        ParametricLocationTree();

        ParametricLocationTree(const ParametricLocation &rootLocation, int maxTime);

        Node getRootNode();

        std::vector<Node> getChildNodes(const Node &parentNode);

        Node setChildNode(const Node parentNode, const ParametricLocation &childLocation);

        int getDimension();

        int getMaxTime();

        void updateRegions();

        void print(bool cummulative);

        void printText();

        std::vector<Node> getCandidateLocationsForTime(double time);

        std::vector<Node> getCandidateLocationsForTimeInterval(std::pair<double,double> interval);  

    }; 
}