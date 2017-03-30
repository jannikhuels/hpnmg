#pragma once

#include "util/Plotter.h"
#include <gmpxx.h>
#include "datastructures/Event.h"
#include "datastructures/Region.h"
#include "exceptions/STDiagramExceptions.h"

using namespace hypro;

namespace hpnmg {
    class STDiagram
    {
    private: 
    
        STDiagram() {};

        static Halfspace<Number> createHalfspaceFromEvent(const Event &event, bool isSourceEvent);

        static bool isValidEvent(const Event &event);

    public:

        static Region getBaseRegion(int dimension, int maxTime);

        static Region createRegion(const Region &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents);

        static void print(const vector<Region> &regionsToPrint, bool cummulative);

    };
}