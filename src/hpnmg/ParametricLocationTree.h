#pragma once

#include <map>
#include <unordered_map>

#include "datastructures/ParametricLocation.h"
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
            void setParametricLocation(const ParametricLocation &location);
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

        vector<pair<string, map<string, float>>> distributions;
    public:
        const vector<pair<string, map<string, float>>> &getDistributions() const;

        void setDistributions(const vector<pair<string, map<string, float>>> &distributions);

    private:

        Region baseRegion;

        void recursivelySetRegions(Node &startNode);

        void recursivelyCollectRegions(const Node &startNode, vector<Region> &regions);

        void recursivelyCollectCandidateLocations(const Node &startNode, vector<Node> &candidates, std::pair<bool, Region> (*isCandidate)(const std::pair<double,double> &interval, const Region &region, int dimension), std::pair<double, double> interval, int dimension);

        void recursivelyCollectCandidateLocationsWithPLT(Node startNode, vector<Node> &candidates, std::pair<double, double> interval, double probability, std::vector<int> occurings);

        std::vector<Event> getSourceEventsFromNodes(const std::vector<Node> &nodes);

        void recursivelyPrintRegions(const ParametricLocationTree::Node &startNode, int depth);

        /**
         * Computes the maximum possible amount of firings for every of the first
         * <code>numberOfGeneralTransitions</code> general transitions from <code>startNode</code> on.
         *
         * For example, a return value of {1, 0, 2} means: For every single path starting in <code>startNode</code>
         * <ul>
         *     <li>the 0th general transition fires at most once</li>
         *     <li>the 1st general transition fires at most zero times, i.e. never</li>
         *     <li>the 2nd general transition fires at most twice</li>
         * </ul>
         *
         * @param startNode
         * @param numberOfGeneralTransitions
         * @return
         */
        std::vector<int> getDimensionRecursively(const ParametricLocationTree::Node &startNode, int numberOfGeneralTransitions);

        /**
         *
         * @param startNode
         * @param genTransOccurings The amount of (globally) maximum firings for every general transition
         * @param dimension
         */
        void addNormedDependenciesRecursively(ParametricLocationTree::Node &startNode, std::vector<int> genTransOccurings, int dimension);

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

        int numberOfLocations();

        std::vector<Node> getCandidateLocationsForTime(double time);

        std::vector<Node> getCandidateLocationsForTimeInterval(std::pair<double,double> interval);  

    }; 
}