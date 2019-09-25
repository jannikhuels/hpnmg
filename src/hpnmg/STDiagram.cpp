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

    std::pair<CONTAINMENT, Region> STDiagram::regionIsCandidateForTime(double time,  const Region &region, int dimension) {
        return STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(time,time), region, dimension);
    }

    std::pair<CONTAINMENT, Region> STDiagram::regionIsCandidateForTimeInterval(const std::pair<double,double> &interval, const Region &region, int dimension) {
        std::pair<matrix_t<double>,vector_t<double>> ihspRepresentations = createHalfspacesFromTimeInterval(interval, dimension);
        std::pair<CONTAINMENT, Region> intersectionPair = region.satisfiesHalfspaces(ihspRepresentations.first, ihspRepresentations.second);
        return intersectionPair;
    }

    Region STDiagram::createTimeRegion(const Region &region, double time, int dimension) {
        Region timeRegion(region);
        std::pair<matrix_t<double>,vector_t<double>> timeHalfspaceRepresentation = createHalfspacesFromTimeInterval(std::pair<double,double>(time,time), dimension);
        std::pair<CONTAINMENT, HPolytope<double>> intersectionPair = region.satisfiesHalfspaces(timeHalfspaceRepresentation.first, timeHalfspaceRepresentation.second);
        return intersectionPair.second;
    }

    Halfspace<double> STDiagram::createHalfspaceForTime(const double &time, int dimension) {
        vector_t<double> direction = vector_t<double>::Zero(dimension);
        direction[dimension-1] = 1;
        return Halfspace<double>(direction, time);
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

    //TODO Check of jannik0general example
    Region STDiagram::intersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> continuousDependencies, double drift, double level, bool negate) {
        Region intersectRegion(baseRegion);
        bool createVerticalHalfspace = false;
        double offset;
        double currentLevel = continuousDependencies[continuousDependencies.size()-1];
        continuousDependencies.pop_back();

        //TODO Check with patricia how these dependencies are created.
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
        std::vector<carl::Interval<double>> boundaries = boundingBox.intervals();
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

    std::vector<Intervals> duplicate(std::vector<Intervals> intervals, carl::Interval<double> res1, carl::Interval<double> res2) {
        std::vector<Intervals> result;
        std::vector<Intervals> part1 = intervals;
        std::vector<Intervals> part2 = intervals;
        part1[0].push_back(res1);
        part2[0].push_back(res2);
        for(int i=1; i<intervals.size();i++) { 
            part1[i].push_back(res1); 
            part2[i].push_back(res2);         
        }
        result.insert(result.end(), part1.begin(), part1.end());
        result.insert(result.end(), part2.begin(), part2.end());     
        return result;
    }

    //TODO Try to solve that inside union
    std::vector<Intervals> STDiagram::removeEmptyIntervals(std::vector<Intervals> intervals) 
    {
        std::vector<Intervals> res;
        for(Intervals I : intervals) {
            bool hasEmpty = false;
            for (int i = 0; i<I.size();i++) {
                if (I[i].isEmpty() || I[i].isZero()) {
                    hasEmpty = true;
                    break;
                } 
            }
            if (!hasEmpty) {
                res.push_back(I);
            }
        } 
        return res;
    }
    
    //TODO put intervals into a single class
    //TODO Check if the empty interval is needed
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
                std::vector<Intervals> res(1); 
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> left;
                    carl::Interval<double> right;
                    //TODO Fix it. This is a workaround due to a bug in carl library
                    if(l[i].upper() == r[i].upper()) {
                        r[i] = carl::Interval<double>(r[i].lower(), r[i].upper()+1);
                    }
                    bool twofold = carl::set_difference(l[i], r[i], left, right);
                    if (twofold) {
                        res = duplicate(res, left, right);                 
                    } else {
                        res[0].push_back(left);
                        for (int i = 1; i < res.size();i++) {
                            res[i].push_back(left);
                        }
                    }                   
                }
                result.insert(result.end(), res.begin(), res.end());
            }             
        } 
        result = STDiagram::removeEmptyIntervals(result);         
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

                double size = l.size();
                Intervals res;
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> intersection;

                    intersection = carl::set_intersection(l[i],r[i]);
                    res.push_back(intersection);                
                }
                result.push_back(res);              
            } 
        }  
        result = STDiagram::removeEmptyIntervals(result);
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
                std::vector<Intervals> res(1);
                for (int i = 0; i < size; i++) {
                    carl::Interval<double> left;
                    carl::Interval<double> right;
                    bool twofold = carl::set_union(l[i], r[i], left, right);
                    if (twofold) {
                        res = duplicate(res, left, right);
                    } else {
                        res[0].push_back(left);
                        for (int i = 1; i < res.size();i++) {
                            res[i].push_back(left);
                        }
                    }                   
                }
                result.insert(result.end(), res.begin(), res.end());
            } 
        }  
        result = STDiagram::removeEmptyIntervals(result);  
        return result;
    }

    std::vector<Region> STDiagram::boundRegionByIntervals(Region reg, int maxTime, std::vector<Intervals> intervals, Halfspace<double> timeHsp) 
    {   
        std::vector<Region> regions;
        Region r = reg;
        for (Intervals i : intervals) {        
            i.push_back(carl::Interval<double>((double)0,(double)maxTime));
            Box<double> box(i);
            Region boxRegion(box.constraints());
            for (Halfspace<double> h: box.constraints()) {
                r.insert(h);
            }
            //Region boxRegion(box.constraints());
            //r.intersect(boxRegion);
            r.insert(timeHsp);
            regions.push_back(r);
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

    Region STDiagram::createRegionForVertices(std::vector<Point<double>> vertices) {
        Region r(vertices);
        return r;
    }

    std::vector<std::vector<double>> STDiagram::stochasticConstraints(Region region) {
        std::vector<std::vector<double>> res;
        for (Halfspace<double> h : region.constraints()) {
            std::vector<double> r;
            r.push_back(h.offset());
            Eigen::VectorXd v = h.normal();
            std::vector<double> coords(v.data(), v.data() + v.rows() * v.cols());
            if (coords.size()>0) {
                r.insert(r.end(), coords.begin(), coords.end()-1);
            }
            res.push_back(r);
        }
        return res;
    }
}