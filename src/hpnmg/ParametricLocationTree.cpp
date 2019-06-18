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

    void ParametricLocationTree::Node::setParametricLocation(const ParametricLocation &parametricLocation) {
        this->parametricLocation = parametricLocation;
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

    void ParametricLocationTree::addNormedDependenciesRecursively(ParametricLocationTree::Node &startNode, std::vector<int> genTransOccurings, int dimension) {
        // get all needed information from parametric location
        ParametricLocation loc = startNode.getParametricLocation();
        vector<int> generalTransitionsFired = loc.getGeneralTransitionsFired();

        const auto normalizeDependencyVector = [dimension, &generalTransitionsFired, &genTransOccurings](std::vector<double> dependencies, double time) {
            // inititalize normed vector and fill with 0
            vector<double> generalDependenciesNormed(dimension, 0);

            // first val in normed dependencies is time
            generalDependenciesNormed[0] = time;

            // startPosition is initial 1 because 0 is the time
            int startPositionForTransition = 1;
            // iterate over all general transitions
            for (int genTrans = 0; genTrans < genTransOccurings.size(); ++genTrans) {
                // iterate over all possible firings of genTrans
                int possibleFirings = genTransOccurings[genTrans];
                int counter = 0;
                for (int realFiring=0; realFiring < generalTransitionsFired.size() && realFiring < dependencies.size(); ++realFiring) {
                    if (generalTransitionsFired[realFiring] == genTrans) { // genTrans had Fired
                        int posNormed = startPositionForTransition + counter;
                        generalDependenciesNormed[posNormed] = dependencies[realFiring];
                        ++counter;
                    }
                }
                startPositionForTransition += possibleFirings;
            }
            assert(startPositionForTransition == dimension);

            return generalDependenciesNormed;
        };

        Event event = loc.getSourceEvent();
        event.setGeneralDependenciesNormed(normalizeDependencyVector(event.getGeneralDependencies(), event.getTime()));
        loc.setSourceEvent(event);

        //region normalize the dependency vectors with time at index 0
        const auto normalizeDependencyVectorWithTime = [&normalizeDependencyVector](std::vector<double> dependencyVector) {
            return normalizeDependencyVector(
                std::vector<double>(dependencyVector.begin() + 1, dependencyVector.end()),
                dependencyVector[0]
            );
        };

        auto continuousMarking = loc.getContinuousMarking();
        std::transform(continuousMarking.begin(), continuousMarking.end(), continuousMarking.begin(), normalizeDependencyVectorWithTime);
        loc.setContinuousMarkingNormed(continuousMarking);

        const auto normalizeGeneralIntervals = [&normalizeDependencyVectorWithTime](auto firings) {
            std::transform(firings.begin(), firings.end(), firings.begin(), normalizeDependencyVectorWithTime);
            return firings;
        };
        auto leftBounds = loc.getGeneralIntervalBoundLeft();
        std::transform(leftBounds.begin(), leftBounds.end(), leftBounds.begin(), normalizeGeneralIntervals);
        loc.setGeneralIntervalBoundNormedLeft(leftBounds);
        auto rightBounds = loc.getGeneralIntervalBoundRight();
        std::transform(rightBounds.begin(), rightBounds.end(), rightBounds.begin(), normalizeGeneralIntervals);
        loc.setGeneralIntervalBoundNormedRight(rightBounds);
        //endregion normalize the dependency vectors with time at index 0

        startNode.setParametricLocation(loc);

        std::pair <std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator, std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator> ret;
        ret = parametricLocations.equal_range(startNode.getNodeID());
        for (std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator it=ret.first; it!=ret.second; ++it) {
            addNormedDependenciesRecursively(it->second, genTransOccurings, dimension);
        }
    }

    //TODO Set Dimension accordingly. This is bad -> the dimension should be equal for the whole tree. And Normed Dependencies it should be
    // set every time tree is computed.
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
            // add normed general dependency vector to all parametric locations
            std::multimap<PARENT_NODE_ID,ParametricLocationTree::Node>::iterator root;
            root = parametricLocations.find(ROOT_NODE_INDEX);
            addNormedDependenciesRecursively(root->second, dimV, dim+1);
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
        recursivelySetRegions(root->second);
    }
    
    void ParametricLocationTree::recursivelySetRegions(ParametricLocationTree::Node &startNode) {
        const auto dimension = this->getDimension();
        const auto &rvIntervals = startNode.getParametricLocation().getRVIntervalsNormed(
            getDimensionRecursively(getRootNode(), startNode.getParametricLocation().getGeneralClock().size()),
            this->maxTime,
            dimension
        );
        Region baseRegion = STDiagram::createBaseRegion(dimension, this->maxTime, rvIntervals);
        std::vector<ParametricLocationTree::Node> childNodes = getChildNodes(startNode);
        if (!childNodes.empty()) {
            startNode.setRegion(STDiagram::createRegion(
                baseRegion,
                startNode.getParametricLocation().getSourceEvent(),
                getSourceEventsFromNodes(childNodes)
            ));

            const auto &ret = parametricLocations.equal_range(startNode.getNodeID());
            for (auto it=ret.first; it!=ret.second; ++it) {
                recursivelySetRegions(it->second);
            }
        } else {
            //TODO Change when a single general transitions fires more than one time
            Region region = STDiagram::createRegionNoEvent(
                baseRegion,
                startNode.getParametricLocation().getSourceEvent(),
                startNode.getParametricLocation().getGeneralIntervalBoundNormedLeft()[0][0],
                startNode.getParametricLocation().getGeneralIntervalBoundNormedRight()[0][0]
            );
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

    bool wayToSortTimes(std::vector<double> a, std::vector<double> b) {
        int aDepIndex = Computation::getDependencyIndex(a);
        int bDepIndex = Computation::getDependencyIndex(b);

        return aDepIndex < bDepIndex;
    }

    void ParametricLocationTree::recursivelyCollectCandidateLocationsWithPLT(Node startNode, vector<Node> &candidates, std::pair<double, double> interval, double probability, std::vector<int> occurings) {
        // nodeProbability is the probability to get here times the probability to be here
        double nodeProbability = startNode.getParametricLocation().getConflictProbability() * probability;
        ParametricLocation parametricLocation = startNode.getParametricLocation();
        parametricLocation.setAccumulatedProbability(nodeProbability);
        int dimension = this->getDimension();

        if (startNode.getParametricLocation().getEarliestEntryTime() <= interval.second) {
            // startNode's earliest entry time is before or equal the questioned time
            // (if it isn't, no childnode can be valid as well!)

            // now we want to check if there is a possible childevent with latest entry time bigger than t.
            const vector<Node> &childNodes = getChildNodes(startNode);

            /*
             * Check how many General Events are fired and sort them by id
             */
            std::vector<double> parentGeneralDependencies = startNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
            std::vector<int> idsOfNextGeneralTransitions;
            for (ParametricLocationTree::Node node : childNodes) {
                std::vector<int> generalTransitionsFired = node.getParametricLocation().getGeneralTransitionsFired();
                if (generalTransitionsFired.size() > parentGeneralDependencies.size()) {
                    idsOfNextGeneralTransitions.push_back(generalTransitionsFired[parentGeneralDependencies.size()]);
                }
            }
            std::sort(idsOfNextGeneralTransitions.begin(), idsOfNextGeneralTransitions.end());
            idsOfNextGeneralTransitions.erase(std::unique(idsOfNextGeneralTransitions.begin(), idsOfNextGeneralTransitions.end()), idsOfNextGeneralTransitions.end());

            bool valid = false;

            std::vector<std::vector<double>> entryTimes;
            std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> bounds;
            auto childs = getChildNodes(startNode);

            // if there is one child location which has a bigger entry time than interval.first, the parent is a candidate
            // if there is no child location, the parent is a candidate as well
            if (getChildNodes(startNode).size()==0) {
                valid = true;
                bounds.push_back(parametricLocation.getRVIntervals(occurings, this->maxTime, dimension));
            }
            for (ParametricLocationTree::Node node : getChildNodes(startNode)) {

                double latestEntryTime = node.getParametricLocation().getLatestEntryTime();
                if (latestEntryTime >= interval.first) {
                    valid = true;

                    /*
                     * Collect the entryTimes of the child node. Beware that we need to ensure that the ordering of each RV of a child location needs to be in accordance to its parent location.
                     */
                    /*std::vector<double> entryTime(dimension);
                    fill(entryTime.begin(), entryTime.end(), 0);
                    entryTime[0] = node.getParametricLocation().getSourceEvent().getTime();

                    std::vector<double> childGeneralDependencies = node.getParametricLocation().getSourceEvent().getGeneralDependencies();
                    std::vector<int> childGeneralTransitionsFired = node.getParametricLocation().getGeneralTransitionsFired();
                    for (int i = 1; i <= parentGeneralDependencies.size(); i++) {
                        entryTime[i] = childGeneralDependencies[i - 1];
                    }
                    if (childGeneralDependencies.size() > parentGeneralDependencies.size()) {
                        int id = childGeneralTransitionsFired[parentGeneralDependencies.size()];
                        int pos =
                                std::find(idsOfNextGeneralTransitions.begin(), idsOfNextGeneralTransitions.end(), id) -
                                idsOfNextGeneralTransitions.begin();
                        entryTime[parentGeneralDependencies.size() + pos +
                                  1] = childGeneralDependencies[parentGeneralDependencies.size()];
                    }
                    entryTimes.push_back(entryTime);*/
                    std::vector<double> entryTime(dimension);
                    fill(entryTime.begin(), entryTime.end(), 0);
                    entryTime[0] = node.getParametricLocation().getSourceEvent().getTime();
                    std::vector<double> childGeneralDependencies = node.getParametricLocation().getSourceEvent().getGeneralDependencies();
                    for (int i = 1; i <= childGeneralDependencies.size(); i++) {
                        entryTime[i] = childGeneralDependencies[i - 1];
                    }
                    entryTimes.push_back(entryTime);
                    bounds.push_back(node.getParametricLocation().getRVIntervals(occurings, this->maxTime, dimension));
                }
            }
            std::vector<std::vector<double>> unsortedEntryTimes = entryTimes;
            std::sort(entryTimes.begin(), entryTimes.end(),  wayToSortTimes);

            if(valid) {
                parametricLocation.setIntegrationIntervals(unsortedEntryTimes, bounds, interval.first, occurings, dimension, this->maxTime);
                startNode.setParametricLocation(parametricLocation);
                candidates.push_back(startNode);
            }

            for (ParametricLocationTree::Node node : getChildNodes(startNode)) {
                recursivelyCollectCandidateLocationsWithPLT(node, candidates, interval, nodeProbability, occurings);
            }

            // not even the earliest entry time is before (or equal) t, so the event happens after the questioned time. no childnode can be valid.
        }
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getCandidateLocationsForTime(double time) {
        return getCandidateLocationsForTimeInterval(std::pair<double,double>{time,time});
    }

    std::vector<ParametricLocationTree::Node> ParametricLocationTree::getCandidateLocationsForTimeInterval(std::pair<double,double> interval) {
        this->getDimension();

        int numberOfGeneralTransitions = getRootNode().getParametricLocation().getGeneralClock().size();
        std::vector<int> dimV = getDimensionRecursively(getRootNode(), numberOfGeneralTransitions);

        vector<ParametricLocationTree::Node> locations;
        // recursivelyCollectCandidateLocations(getRootNode(), locations, &STDiagram::regionIsCandidateForTimeInterval, interval, this->getDimension());
        recursivelyCollectCandidateLocationsWithPLT(getRootNode(), locations, interval, 1.0, dimV);
        return locations;
    }

    /**
     * @return std::vector where the i-th entry representes the probability distribution of the i-th general transition.
     */
    const vector<pair<string, map<string, float>>> &hpnmg::ParametricLocationTree::getDistributions() const {
        return distributions;
    }

    /**
     * @return std::vector where the i-th entry representes the probability distribution of the i-th random variable,
     *         that is the i-th firing in the normalized global firing order
     */
    vector<pair<string, map<string, float>>> hpnmg::ParametricLocationTree::getDistrbutionsNormalized() {
        auto distributionsNormalized = vector<pair<string, map<string, float>>>();
        distributionsNormalized.reserve(this->getDimension());

        const auto &occurrings = this->getDimensionRecursively(this->getRootNode(), getRootNode().getParametricLocation().getGeneralClock().size());
        auto it = distributionsNormalized.begin();
        for (int transition = 0; transition < occurrings.size(); ++transition)
            for (int i = 0; i < occurrings[transition]; ++i)
                distributionsNormalized.push_back(this->getDistributions()[transition]);

        return distributionsNormalized;
    }

    void
    hpnmg::ParametricLocationTree::setDistributions(const vector<pair<string, map<string, float>>> &distributions) {
        ParametricLocationTree::distributions = distributions;
    }

    int hpnmg::ParametricLocationTree::numberOfLocations() {
        return this->parametricLocations.size();
    }
}
