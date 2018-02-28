#include "STDiagram.h"

namespace hpnmg {

    Region STDiagram::createBaseRegion(int dimension, int maxTime) {
        if (dimension < 1)
            throw IllegalDimensionException(dimension);
        if (maxTime < 1) {
            throw IllegalMaxTimeException(maxTime);
        }

        std::vector<Halfspace<double>> box;

        for (int i = 0; i < dimension; i++) {
            vector_t<double> direction = vector_t<double>::Zero(dimension);
            direction[i] = -1;
  
            Halfspace<double> lowerHsp(direction,0);
            box.emplace_back(lowerHsp);

            Halfspace<double> upperHsp(-direction,maxTime);
            box.emplace_back(upperHsp);            
        }

        Region diagramPolytope(box);
        return diagramPolytope;
    }

    Region STDiagram::createRegion(const Region &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents) {
        Region region(baseRegion);

        if (!isValidEvent(sourceEvent)) {
            throw IllegalArgumentException("sourceEvent");
        }

        region.insert(createHalfspaceFromEvent(sourceEvent,true));

        for (Event e : destinationEvents) {
            if (!isValidEvent(e)) {
                throw IllegalArgumentException("destinationEvents");
            }
            region.insert(createHalfspaceFromEvent(e, false));
        }
        return region;
    }

    Halfspace<double> STDiagram::createHalfspaceFromDependencies(std::vector<double> dependencies, double offset, bool isVertical) {
        vector_t<double> direction = vector_t<double>::Zero(dependencies.size()+1);
        int i = 0;
        for (; i < dependencies.size(); i++) {
            direction[i] = dependencies[i];
        }
        if (!isVertical) {
            direction[i] = 1;
        }
        

        Halfspace<double> hsp(direction, offset);
        
        return hsp;
    }

    Halfspace<double> STDiagram::createHalfspaceFromEvent(const Event &event, bool isSourceEvent) {
        Halfspace<double> hsp = STDiagram::createHalfspaceFromDependencies(event.getGeneralDependencies(), event.getTime(), false);
        
        if (isSourceEvent)
            return -hsp;
        return hsp;
    }

    bool STDiagram::isValidEvent(const Event &event) {
        if (!(event.getGeneralDependencies().size() > 0)) { 
            return false;
        }
        return true;
    }

    void STDiagram::print(const vector<Region> &regionsToPrint, bool cummulative) {
        Plotter<double>& plt = Plotter<double>::getInstance();
        plt.rSettings().cummulative = cummulative;

        for (Region region : regionsToPrint) {
            plt.addObject(region.vertices());
        }

        plt.plot2d();
    }

    bool STDiagram::regionIsCandidateForTime(double time,  const Region &region, int dimension) {
        return STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(time,time), region, dimension);
    }

    bool STDiagram::regionIsCandidateForTimeInterval(const std::pair<double,double> &interval, const Region &region, int dimension) {
        std::pair<matrix_t<double>,vector_t<double>> ihspRepresentations = createHalfspacesFromTimeInterval(interval, dimension);
        std::pair<bool, HPolytope<double>> intersectionPair = region.satisfiesHalfspaces(ihspRepresentations.first, ihspRepresentations.second);
        return intersectionPair.first;
    }

    Region STDiagram::createTimeRegion(const Region &region, double time, int dimension) {
        Region timeRegion(region);
        std::pair<matrix_t<double>,vector_t<double>> timeHalfspaceRepresentation = createHalfspacesFromTimeInterval(std::pair<double,double>(time,time), dimension);
        std::pair<bool, HPolytope<double>> intersectionPair = region.satisfiesHalfspaces(timeHalfspaceRepresentation.first, timeHalfspaceRepresentation.second);
        return intersectionPair.second;
    }

    std::pair<matrix_t<double>,vector_t<double>> STDiagram::createHalfspacesFromTimeInterval(const std::pair<double, double> &interval, int dimension) { 
        if (interval.first < 0) {
            throw IllegalArgumentException("interval", "Start time < 0.");
        }
        if (interval.second < interval.first) {
            throw IllegalArgumentException("interval", "Start time < Stop time.");
        }
        if (dimension < 2) {
            throw IllegalArgumentException("dimension", "Dimension should be >= 2.");
        }
        matrix_t<double> lineMat = matrix_t<double>::Zero(2,dimension);
        vector_t<double> lineVec = vector_t<double>::Zero(2);
        lineMat(0,dimension-1) = 1;
        lineMat(1,dimension-1) = -1;
        lineVec << interval.second,-interval.first;
        return std::pair<matrix_t<double>,vector_t<double>>(lineMat, lineVec); 
    }

    Region STDiagram::createRegionNoEvent(const Region &baseRegion, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds) {
        Region region(baseRegion);

        if (!isValidEvent(sourceEvent)) {
            throw IllegalArgumentException("sourceEvent");
        }

        region.insert(createHalfspaceFromEvent(sourceEvent,true));
        if (leftBounds.size() > 0)
            region.insert(createVerticalHalfspace(leftBounds, true));
        if (rightBounds.size() > 0)
            region.insert(createVerticalHalfspace(rightBounds, false));
            
        return region;
    }

    vector_t<double> STDiagram::vectorFromBound(double bound, int boundIndex, int dimension) {
        vector_t<double> vector = vector_t<double>::Zero(dimension);
        vector[boundIndex] = bound;
        return vector;
    }

    std::vector<vector_t<double>> STDiagram::hspVectorsFromBounds(std::vector<double> bounds) {
        std::vector<vector_t<double>> hspVectors; 
        int dimension = bounds.size() + 1;
        for (int i = 0; i < bounds.size() ; i++) {
            vector_t<double> vectorBottom = vectorFromBound(bounds.at(i), i, dimension);
            vector_t<double> vectorTop = vectorFromBound(bounds.at(i), i, dimension);
            vectorTop[dimension-1] = 1;
            hspVectors.push_back(vectorBottom);
            hspVectors.push_back(vectorTop);
        }
        return hspVectors;
    }

    std::vector<vector_t<double>> STDiagram::directionVectorsFromHspVectors(std::vector<vector_t<double>> hspVectors) {
        std::vector<vector_t<double>> directionVectors;
        for(int i = 1; i < hspVectors.size(); i++) {
            directionVectors.push_back(hspVectors.at(i) - hspVectors.at(0));
        }
        return directionVectors;
    }

    Halfspace<double> STDiagram::createVerticalHalfspace(std::vector<double> bounds, bool isLeftBound) {
        int dimension = bounds.size() + 1;

        std::vector<vector_t<double>> directionVectors = directionVectorsFromHspVectors(hspVectorsFromBounds(bounds));
        vector_t<double> planeNormal = Halfspace<double>::computePlaneNormal(directionVectors);

        vector_t<double> planePointVector = vectorFromBound(bounds.at(0), 0, dimension);
        double planeOffset = Halfspace<double>::computePlaneOffset(planeNormal, Point<double>(planePointVector));

        Halfspace<double> hsp(planeNormal,planeOffset);

        if (isLeftBound)
            return -hsp;
        return hsp;
    }

    Region STDiagram::intersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> continuousDependencies, double drift, double level) {
        Region intersectRegion(baseRegion);
        bool createVerticalHalfspace = false;
        double offset;
        double currentLevel = continuousDependencies[continuousDependencies.size()-1];
        continuousDependencies.pop_back();
        double divisor = drift;

        if (drift == 0) {
            offset = level - currentLevel;
            createVerticalHalfspace = true;
            divisor = 1;
        } else {
            offset = (level-currentLevel)/drift;
        }    

        bool hasDependency = false;
        for (int i = 0; i < continuousDependencies.size(); i++) {
            if (continuousDependencies[i] != 0) {
                hasDependency = true;
            }
            continuousDependencies[i] = continuousDependencies[i]/divisor;
        }   

        if (hasDependency == false) {
            if (drift == 0) {
                throw IllegalIntersectionLevelException();
            }
        }    

        Halfspace<double> fillLevelHsp = STDiagram::createHalfspaceFromDependencies(continuousDependencies, offset, createVerticalHalfspace);
        if (drift < 0)
            fillLevelHsp = -fillLevelHsp;

        intersectRegion.insert(fillLevelHsp);
        return intersectRegion;
    }

}