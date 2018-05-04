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

        /*if (!isValidEvent(sourceEvent, baseRegion)) {
            throw IllegalArgumentException("sourceEvent");
        }*/
        region.insert(createHalfspaceFromEvent(makeValidEvent(sourceEvent, baseRegion),true));
        
        for (Event e : destinationEvents) {
            /*if (!isValidEvent(e, baseRegion)) {
                throw IllegalArgumentException("destinationEvents");
            }*/
            region.insert(createHalfspaceFromEvent(makeValidEvent(e, baseRegion), false));
        }
        return region;
    }

    Halfspace<double> STDiagram::createHalfspaceFromDependencies(std::vector<double> dependencies, double offset, bool isVertical) {
        vector_t<double> direction = vector_t<double>::Zero(dependencies.size()+1);
        int i = 0;
        for (; i < dependencies.size(); i++) {
            direction[i] = -dependencies[i];
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

    // TODO This is wrong - should not be needed
    std::vector<double> STDiagram::makeValidDependencies(std::vector<double> dependencies, int dimension) {
        auto gdependencies = dependencies;
        for (int i = gdependencies.size(); i < dimension - 1; i++) {
            gdependencies.push_back(0);
        }
        return gdependencies;
    }

    Event STDiagram::makeValidEvent(const Event &event, const Region &baseRegion) {
        Event e(event);
        if (!(e.getGeneralDependencies().size() == baseRegion.dimension()-1)) { 
            auto gdependencies = STDiagram::makeValidDependencies(event.getGeneralDependencies(), baseRegion.dimension());
            e.setGeneralDependencies(gdependencies);                  
        }
        return e;
    }

    void STDiagram::print(const vector<Region> &regionsToPrint, bool cummulative, std::string filename) {
        Plotter<double>& plt = Plotter<double>::getInstance();
        plt.clear();
        plt.setFilename(filename);
        plt.rSettings().cummulative = cummulative;

        for (Region region : regionsToPrint) {
            plt.addObject(region.vertices());
        }

        plt.plot2d();
    }

    std::pair<bool, Region> STDiagram::regionIsCandidateForTime(double time,  const Region &region, int dimension) {
        return STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(time,time), region, dimension);
    }

    std::pair<bool, Region> STDiagram::regionIsCandidateForTimeInterval(const std::pair<double,double> &interval, const Region &region, int dimension) {
        std::pair<matrix_t<double>,vector_t<double>> ihspRepresentations = createHalfspacesFromTimeInterval(interval, dimension);
        std::pair<bool, Region> intersectionPair = region.satisfiesHalfspaces(ihspRepresentations.first, ihspRepresentations.second);
        return intersectionPair;
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

    Region STDiagram::createRegionNoEvent(const Region &r, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds) {
        Region region(r);

        /*if (!isValidEvent(sourceEvent, baseRegion)) {
            throw IllegalArgumentException("sourceEvent");
        }*/

        region.insert(createHalfspaceFromEvent(makeValidEvent(sourceEvent, r),true));
        if (leftBounds.size() > 0)
            region.insert(createVerticalHalfspace(makeValidDependencies(leftBounds, r.dimension()), true));
        if (rightBounds.size() > 0)
            region.insert(createVerticalHalfspace(makeValidDependencies(rightBounds, r.dimension()), false));
            
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

    Region STDiagram::intersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> continuousDependencies, double drift, double level, bool negate) {
        Region intersectRegion(baseRegion);
        bool createVerticalHalfspace = false;
        double offset;
        double currentLevel = continuousDependencies[continuousDependencies.size()-1];
        continuousDependencies.pop_back();

        continuousDependencies = STDiagram::makeValidDependencies(continuousDependencies, baseRegion.dimension());

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
                return Region::Empty();
            }
        }    

        Halfspace<double> fillLevelHsp = STDiagram::createHalfspaceFromDependencies(continuousDependencies, offset, createVerticalHalfspace);
        if (drift < 0)
            fillLevelHsp = -fillLevelHsp;

        if (negate)
            fillLevelHsp = -fillLevelHsp;

        intersectRegion.insert(fillLevelHsp);
        return intersectRegion;
    }

    Intervals STDiagram::createIntervals(const Region &baseInterval)
    {
        //Region base = STDiagram::createBaseRegion(baseInterval.dimension(), time);
        Box<double> boundingBox = Converter<double>::toBox(baseInterval, CONV_MODE::ALTERNATIVE);
        std::vector<carl::Interval<double>> boundaries = boundingBox.boundaries();
        /*printf("Number of intervals: %lu\n", boundaries.size());
        printf("Intervals:\n");
        for (carl::Interval<double> i : boundaries) {
            printf("[%f,%f]\n", i.lower(), i.upper());
        }*/
        if(boundingBox.dimension() == baseInterval.dimension()) {
            boundaries.pop_back();
        }
        //carl::Interval<double> timeInterval = carl::Interval<double>((double)0,time);
        //boundaries.push_back(timeInterval);
        //printf("New boundaries size: %lu\n", boundaries.size());
        //TODO intersect Region with lower halfspace.
        return boundaries;
    }

    std::vector<Intervals> STDiagram::differenceOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2) 
    {
        std::vector<Intervals> result;
        for (Intervals l : i1) {
            if (i2.size() == 0) {
                return i1;
            }
            for (Intervals r : i2) {
                if (l.size() != r.size()) {
                    //TODO THis is an error
                }

                double size = l.size();
                Intervals res1;
                Intervals res2;
                bool hasEmptyInterval = false;
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> left;
                    carl::Interval<double> right;
                    //TODO Fix it. This is a workaround due to a bug in carl library
                    if(l[i].upper() == r[i].upper()) {
                        r[i] = carl::Interval<double>(r[i].lower(), r[i].upper()+1);
                    }
                    bool twofold = l[i].difference(r[i], left, right);
                    if (twofold && !right.isEmpty()) {
                        res2.push_back(right);
                    }
                    if (!left.isEmpty()) {
                        res1.push_back(left);
                        continue;
                    }          
                    hasEmptyInterval = true;          
                }
                if (!hasEmptyInterval) {
                    if (res1.size()>0) {
                        result.push_back(res1);
                    }                
                    if (res2.size()>0) {
                        result.push_back(res2);
                    }
                }
            }             
        }          
        return result;
    }

    std::vector<Intervals> STDiagram::intersectionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2)
    {
        std::vector<Intervals> result;
        for (Intervals l : i1) {
            if (i2.size() == 0) {
                return result;
            }
            for (Intervals r : i2) {
                if (l.size() != r.size()) {
                    //TODO This is an error
                }

                bool hasEmptyInterval = false;
                double size = l.size();
                Intervals res;
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> intersection;

                    intersection = l[i].intersect(r[i]);
                    
                    if (!intersection.isEmpty()) {
                        res.push_back(intersection);
                        continue;
                    }   
                    hasEmptyInterval = true;                 
                }
                if (!hasEmptyInterval && res.size()>0) {
                    result.push_back(res);
                }                
            } 
        }    
        return result;
    }

    std::vector<Intervals> STDiagram::unionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2)
    {
        std::vector<Intervals> result;
        if (i1.size() == 0) {
            return i2;        
        }
        for (Intervals l : i1) {
            if (i2.size() == 0) {
                return i1;
            }
            for (Intervals r : i2) {
                if (l.size() != r.size()) {
                    //TODO THis is an error
                }

                double size = l.size();
                Intervals res1;
                Intervals res2;
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> left;
                    carl::Interval<double> right;
                    cout << r[i] << endl;
                    bool twofold = l[i].unite(r[i], left, right);
                    if (twofold && !right.isEmpty()) {
                        res2.push_back(right);
                    }
                    if (!left.isEmpty()) {
                        res1.push_back(left);
                    }                    
                }
                if (res1.size()>0) {
                    result.push_back(res1);
                }                
                if (res2.size()>0) {
                    result.push_back(res2);
                }
            } 
        }    
        return result;
    }

    std::vector<Region> STDiagram::boundRegionByIntervals(Region r, std::vector<Intervals> intervals, double time) 
    {   
        std::vector<Region> regions;
        for (Intervals i : intervals) {
            i.push_back(carl::Interval<double>((double)0,time));
            Box<double> box(i);
            Region boxRegion(box.constraints());
            regions.push_back(r.intersect(boxRegion));
        }
        return regions;
    }

    std::vector<Intervals> STDiagram::intervalsFromRegions(std::vector<Region> regions) {
        std::vector<Intervals> result;
        for (Region r : regions) {
            result.push_back(STDiagram::createIntervals(r));
        }
        return result;
    }
}