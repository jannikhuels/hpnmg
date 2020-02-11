#include "STDiagram.h"
#include "util/statistics/Statistics.h"
#include "carl/interval/Interval.h"
#include "carl/interval/set_theory.h"

namespace hpnmg {

    /**
     * Create a hypercubic region with the provided dimension.
     *
     * @param dimension At least 1
     * @param maxTime At least 1
     * @return The <code>dimension</code>-fold cartesian product of <code>[0, maxTime]</code>
     */
    STDPolytope<double> STDiagram::createBaseRegion(int dimension, int maxTime) {
        return createBaseRegion(dimension, maxTime, {});
    }

    /**
     * First creates a hypercubic region with the provided dimension limited by <code>maxTime</code> in all directions.
     * Then further limits that region by applying the <code>rvIntervals</code> to each dimension where the n-th element
     * limits the n-th dimension.
     *
     * If <code>dimension</code> is greater than <code>rvIntervals.size()</code>, the remaining dimensions are not
     * limited by anything other than the initial bounding box. This is usually used to add a time-dimension after all
     * random variable domains.
     *
     * @param dimension At least 1 and at least as large as <code>rvIntervals.size()</code>
     * @param maxTime At least 1
     * @param rvIntervals Extra restrictions for the dimensions beyond the initial interval <code>[0, maxTime]</code>.
     *                    May contain fewer elements than there are dimensions.
     * @return
     */
    STDPolytope<double> STDiagram::createBaseRegion(int dimension, int maxTime, const std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &rvIntervals) {
        if (maxTime < 1)
            throw IllegalMaxTimeException(maxTime);
        if (dimension < 1 || dimension < rvIntervals.size())
            throw IllegalDimensionException(dimension);

        STDPolytope<double>::Polytope polytope{};

        // Ensure that the polytope is properly limited by the time in every dimension
        for (int currentDimension = 0; currentDimension < dimension; ++currentDimension)
        {
            hypro::vector_t<double> dimensionDirection = hypro::vector_t<double>::Zero(dimension);
            dimensionDirection[currentDimension] = 1;
            polytope.insert(Halfspace<double>(-dimensionDirection, 0));
            polytope.insert(Halfspace<double>(dimensionDirection, maxTime));
        }

        // Each RV corresponds to one dimension of the region. We'll restrict the polytope accordingly.
        for (int currentRV = 0; currentRV < rvIntervals.size(); ++currentRV) {
            const auto &firing = rvIntervals[currentRV];
            const auto &lowerBound = firing.second.first;
            const auto &upperBound = firing.second.second;

            assert(lowerBound.size() == upperBound.size());
            // The dimension must be large enough to include all random variables
            assert(dimension >= lowerBound.size() - 1);
            if (!lowerBound.empty()) {
                hypro::vector_t<double> lowerDirection = hypro::vector_t<double>::Zero(dimension);
                hypro::vector_t<double> upperDirection = hypro::vector_t<double>::Zero(dimension);

                double lowerOffset = lowerBound[0];
                lowerDirection[currentRV] = 1;
                double upperOffset = upperBound[0];
                upperDirection[currentRV] = 1;

                for (int dependencyRV = 0; dependencyRV + 1 < lowerBound.size(); ++dependencyRV) {
                    // Move the coefficients from the right side of the (in)equation to the left side
                    lowerDirection[dependencyRV] -= lowerBound[dependencyRV + 1];
                    upperDirection[dependencyRV] -= upperBound[dependencyRV + 1];
                }

                polytope.insert(Halfspace<double>(-lowerDirection, -lowerOffset));
                polytope.insert(Halfspace<double>(upperDirection, upperOffset));
            }
        }

        return STDPolytope<double>(polytope);
    }

    STDPolytope<double> STDiagram::createRegion(const STDPolytope<double> &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents) {
        STDPolytope<double> region(baseRegion);

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

    Halfspace<double> STDiagram::createHalfspaceFromDependenciesNormed(std::vector<double> dependencies) {
        vector_t<double> direction = vector_t<double>::Zero(dependencies.size());
        // dependenciesNormed carry the time as first element so we have to copy all entries but that one ...
        for (int i = 1; i < dependencies.size(); i++) {
            direction[i - 1] = -dependencies[i];
        }
        // ... and set the coefficient for the time-dimension to 1
        direction[dependencies.size() - 1] = 1;

        Halfspace<double> hsp(direction, dependencies[0]);

        return hsp;
    }

    Halfspace<double> STDiagram::createHalfspaceFromEvent(const Event &event, bool isSourceEvent) {
        Halfspace<double> hsp = STDiagram::createHalfspaceFromDependenciesNormed(event.getGeneralDependenciesNormed());

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

    Event STDiagram::makeValidEvent(const Event &event, const STDPolytope<double> &baseRegion) {
        Event e(event);
        if (!(e.getGeneralDependencies().size() == baseRegion.dimension()-1)) {
            auto gdependencies = STDiagram::makeValidDependencies(event.getGeneralDependencies(), baseRegion.dimension());
            e.setGeneralDependencies(gdependencies);
        }
        return e;
    }

    void STDiagram::print(const vector<STDPolytope<double>> &regionsToPrint, bool cummulative, std::string filename) {
        Plotter<double>& plt = Plotter<double>::getInstance();
        plt.clear();
        plt.setFilename(filename);
        plt.rSettings().cummulative = cummulative;

        for (STDPolytope<double> region : regionsToPrint)
            region.addToPlot(plt);

        plt.plot2d();
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

    STDPolytope<double> STDiagram::createRegionNoEvent(const STDPolytope<double> &r, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds) {
        STDPolytope<double> region(r);

        /*if (!isValidEvent(sourceEvent, baseRegion)) {
            throw IllegalArgumentException("sourceEvent");
        }*/

        region.insert(createHalfspaceFromEvent(makeValidEvent(sourceEvent, r),true));

        return region;
    }

    std::vector<vector_t<double>> STDiagram::directionVectorsFromHspVectors(std::vector<vector_t<double>> hspVectors) {
        std::vector<vector_t<double>> directionVectors;
        for(int i = 1; i < hspVectors.size(); i++) {
            directionVectors.push_back(hspVectors.at(i) - hspVectors.at(0));
        }
        return directionVectors;
    }

    //TODO Check of jannik0general example
    STDPolytope<double> STDiagram::legacyIntersectRegionForContinuousLevel(const STDPolytope<double> &baseRegion, std::vector<double> continuousDependencies, double drift, double level, bool negate) {
        STDPolytope<double> intersectRegion(baseRegion);
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
                return STDPolytope<double>::Empty(baseRegion.dimension());
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

    STDPolytope<double> STDiagram::intersectRegionForContinuousLevel(const STDPolytope<double> &baseRegion, std::vector<double> entryTimeNormed, std::vector<double> markingNormed, double drift, double level, bool negate) {
        assert(markingNormed.size() == entryTimeNormed.size());
        COUNT_STATS("STDIAGRAM_INTERSECT_REGION_CONTINUOUS_LEVEL")
        START_BENCHMARK_OPERATION("STDIAGRAM_INTERSECT_REGION_CONTINUOUS_LEVEL")
        vector_t<double> markingDirection = vector_t<double>::Zero(markingNormed.size());
        vector_t<double> entryTimeDirection = vector_t<double>::Zero(entryTimeNormed.size());

        // normed dependencies carry the time as first element so we have to copy all entries but that one
        for (int i = 1; i < markingNormed.size(); i++) {
            markingDirection[i - 1] = markingNormed[i];
            entryTimeDirection[i - 1] = entryTimeNormed[i];
        }

        // The level dependent on the RV-valuation s and time t ("initial marking + drift * time since entry"):
        // markingNormed(s) + drift * (t - entryTimeNormed(s))
        // We transform the equation to something like:
        // (markingNormed - drift * entryTimeNormed)(s) + drift * t
        auto direction = vector_t<double>(markingDirection - (drift * entryTimeDirection));
        // The hypro vectors have the time-coefficient at the last index.
        direction[direction.size() - 1] = drift;
        // While the actual time-offsets are provided separately
        const auto satHalfspace = Halfspace<double>(direction, level + (drift * entryTimeNormed[0]) - markingNormed[0]);

        if (direction.isZero()) {
            // The level is independent of RVs and time, thus constant in the region. Check if it satisfies the level.
            if ((markingNormed[0] <= level) != negate) {
                STOP_BENCHMARK_OPERATION("STDIAGRAM_INTERSECT_REGION_CONTINUOUS_LEVEL")
                return baseRegion;
            }
            else {
                STOP_BENCHMARK_OPERATION("STDIAGRAM_INTERSECT_REGION_CONTINUOUS_LEVEL")
                return STDPolytope<double>::Empty(baseRegion.dimension());
            }
        }

        STDPolytope<double> intersectRegion(baseRegion);
        if (!negate)
            intersectRegion.insert(satHalfspace);
        else
            intersectRegion.insertOpen(-satHalfspace);
        STOP_BENCHMARK_OPERATION("STDIAGRAM_INTERSECT_REGION_CONTINUOUS_LEVEL")
        return intersectRegion;
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
                    //bool twofold = l[i].difference(r[i], left, right);
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

                    //intersection = l[i].intersect(r[i]);
                    carl::set_intersection(l[i], r[i]);
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
                    //bool twofold = l[i].unite(r[i], left, right);
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

    std::vector<STDPolytope<double>> STDiagram::boundRegionByIntervals(STDPolytope<double> reg, int maxTime, std::vector<Intervals> intervals, Halfspace<double> timeHsp)
    {
        std::vector<STDPolytope<double>> regions;
        STDPolytope<double> r = reg;
        for (Intervals i : intervals) {
            i.push_back(carl::Interval<double>((double)0,(double)maxTime));
            Box<double> box(i);
            STDPolytope<double> boxRegion(box.constraints());
            for (Halfspace<double> h: box.constraints()) {
                r.insert(h);
            }
            //STDPolytope<double> boxRegion(box.constraints());
            //r.intersect(boxRegion);
            r.insert(timeHsp);
            regions.push_back(r);
        }
        return regions;
    }

    STDPolytope<double> STDiagram::createRegionForVertices(std::vector<Point<double>> vertices) {
        STDPolytope<double> r(vertices);
        return r;
    }

}