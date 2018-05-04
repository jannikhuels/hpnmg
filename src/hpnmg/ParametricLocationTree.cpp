#include "ParametricLocationTree.h"
#include <numeric>

namespace hpnmg {

    ParametricLocationTree::Node::Node(NODE_ID id, const ParametricLocation &parametricLocation) : id(id), parametricLocation(parametricLocation) {
        
    }

    NODE_ID ParametricLocationTree::Node::getNodeID() const {return id;}
    ParametricLocation ParametricLocationTree::Node::getParametricLocation() const {return parametricLocation;}
    Region ParametricLocationTree::Node::getRegion() const {return region;}

    void ParametricLocationTree::Node::setRegion(const Region &region) {
        this->region = region;
    }

    ParametricLocationTree::ParametricLocationTree(const ParametricLocation &rootLocation, int maxTime) : currentId(ROOT_NODE_ID), maxTime(maxTime), baseRegion(STDiagram::createBaseRegion(rootLocation.getDimension(), maxTime)) {
        setRootNode(rootLocation);
        this->dimension = 0;
    }

    ParametricLocationTree::ParametricLocationTree() {
        this->dimension = 0;
    }

    ParametricLocationTree::Node ParametricLocationTree::getRootNode() {
        return getNodes(ROOT_NODE_INDEX)[0];
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getChildNodes(const ParametricLocationTree::Node &parentNode) {
        if (parentNode.getNodeID() >= ROOT_NODE_ID) {
            return getNodes(parentNode.getNodeID());
        } 
        throw InvalidParentNodeException();
    }

    ParametricLocationTree::Node ParametricLocationTree::setChildNode(const ParametricLocationTree::Node parentNode, const ParametricLocation &childLocation) {
        return insertParametricLocation(parentNode.getNodeID(), childLocation);
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getNodes(PARENT_NODE_ID parentID) {
        typedef std::multimap<PARENT_NODE_ID, Node>::iterator mmiterator;
        std::vector<ParametricLocationTree::Node> nodes;

        std::pair <mmiterator, mmiterator> nodeRange;
        nodeRange = parametricLocations.equal_range(parentID);

        for (mmiterator iterator = nodeRange.first; iterator != nodeRange.second; iterator++) {
            nodes.push_back(iterator->second);
        }
        return nodes;
    }

    std::vector<int> ParametricLocationTree::getDimensionRecursively(const ParametricLocationTree::Node &startNode, int numberOfGeneralTransitions) {
        ParametricLocation loc = startNode.getParametricLocation();
        std::vector<int> current(numberOfGeneralTransitions);
        for (int i = 0; i < numberOfGeneralTransitions; i++) {
            for (int j : loc.getGeneralTransitionsFired()) {
                if (j==i) {
                    current[i]++;
                }
            }      
        }
        int dim = (int)startNode.getParametricLocation().getGeneralTransitionsFired().size();
        for (ParametricLocationTree::Node node : getChildNodes(startNode)) {
            std::vector<int> cMaxima = getDimensionRecursively(node, numberOfGeneralTransitions);
            for (int i = 0; i < numberOfGeneralTransitions; i++) {
                if (current[i] < cMaxima[i]) {
                    current[i] = cMaxima[i];
                }
            }         
        }
        return current;
    }

    //TODO Set Dimension accordingly. This is bad -> the dimension should be equal for the whole tree.
    int ParametricLocationTree::getDimension() { 
        if (this->dimension == 0) {
            int numberOfGeneralTransitions = getRootNode().getParametricLocation().getGeneralClock().size();
            std::vector<int> dimV = getDimensionRecursively(getRootNode(), numberOfGeneralTransitions);
            int dim = 0;
            for (int i = 0; i < dimV.size(); i++) {
                dim += dimV[i];
            }
            if (dim == 0) {
                this->dimension = getRootNode().getParametricLocation().getDimension();
            } else {
                this->dimension = dim + 1;
            }            
        }
        return this->dimension;
    }

    int ParametricLocationTree::getMaxTime() {
        return maxTime;
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

    ParametricLocationTree::Node ParametricLocationTree::insertParametricLocation(PARENT_NODE_ID parentNodeId, const ParametricLocation &parametricLocation) {
        if (parentNodeId >= ROOT_NODE_INDEX && parentNodeId < currentId) {
            ParametricLocationTree::Node newNode(currentId, parametricLocation);
            parametricLocations.insert(std::pair<int, Node>(parentNodeId,newNode));
            currentId++;
            return newNode;
        } else {
            throw InvalidParentNodeException();
        }
    }

    void ParametricLocationTree::updateRegions() {
        std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator root;
        root = parametricLocations.find(ROOT_NODE_INDEX);
        Region r = STDiagram::createBaseRegion(this->getDimension(), this->maxTime);
        recursivelySetRegions(root->second, r);
    }
    
    void ParametricLocationTree::recursivelySetRegions(ParametricLocationTree::Node &startNode, Region &r) {
        std::vector<ParametricLocationTree::Node> childNodes = getChildNodes(startNode);
        if (childNodes.size() > 0) {
            Region region = STDiagram::createRegion(r, startNode.getParametricLocation().getSourceEvent(), getSourceEventsFromNodes(childNodes));
            startNode.setRegion(region);

            std::pair <std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator, std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator> ret;
            ret = parametricLocations.equal_range(startNode.getNodeID());
            for (std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator it=ret.first; it!=ret.second; ++it) {
                recursivelySetRegions(it->second, r);
            }
        } else {
            //TODO Change when a single general transitions fires more than one time
            Region region = STDiagram::createRegionNoEvent(r, startNode.getParametricLocation().getSourceEvent(), startNode.getParametricLocation().getGeneralIntervalBoundLeft()[0][0], startNode.getParametricLocation().getGeneralIntervalBoundRight()[0][0]);
            startNode.setRegion(region);
        }
    }

    std::vector<Event> ParametricLocationTree::getSourceEventsFromNodes(const std::vector<ParametricLocationTree::Node> &nodes) {
        std::vector<Event> events;
        for (ParametricLocationTree::Node node : nodes) {
            events.push_back(node.getParametricLocation().getSourceEvent());
        }
        return events;
    }

    void ParametricLocationTree::recursivelyCollectRegions(const ParametricLocationTree::Node &startNode, vector<Region> &regions) {
        regions.push_back(startNode.getRegion());
        for (ParametricLocationTree::Node node : getChildNodes(startNode)) {
            recursivelyCollectRegions(node, regions);
        }
    }
    
    void ParametricLocationTree::print(bool cummulative) {
        vector<Region> regions;
        recursivelyCollectRegions(getRootNode(), regions);
        STDiagram::print(regions, cummulative);
    }

    void ParametricLocationTree::recursivelyPrintRegions(const ParametricLocationTree::Node &startNode, int depth) {
        for (int i = 0; i<depth; i++) {
            printf(" ");
        }

        startNode.getParametricLocation().getSourceEvent().print();

        for (ParametricLocationTree::Node node : getChildNodes(startNode)) {      
            recursivelyPrintRegions(node, depth+1);
        }
    }

    void ParametricLocationTree::printText() {
        recursivelyPrintRegions(getRootNode(),0);
    }

    void ParametricLocationTree::recursivelyCollectCandidateLocations(const Node &startNode, vector<Node> &candidates, std::pair<bool, Region> (*isCandidate)(const std::pair<double,double> &interval, const Region &region, int dimension), std::pair<double, double> interval, int dimension) {
        if (isCandidate(interval, startNode.getRegion(), dimension).first) {
            candidates.push_back(startNode);
        }
        for (ParametricLocationTree::Node node : getChildNodes(startNode)) {
            recursivelyCollectCandidateLocations(node, candidates, isCandidate, interval, dimension);
        }
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getCandidateLocationsForTime(double time) {
        return getCandidateLocationsForTimeInterval(std::pair<double,double>{time,time});
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getCandidateLocationsForTimeInterval(std::pair<double,double> interval) {
        vector<ParametricLocationTree::Node> locations;
        recursivelyCollectCandidateLocations(getRootNode(), locations, &STDiagram::regionIsCandidateForTimeInterval, interval, this->getDimension());
        return locations;
    }
}
