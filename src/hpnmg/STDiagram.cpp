#include "STDiagram.h"

namespace hpnmg {

    Region STDiagram::getBaseRegion(int dimension, int maxTime) {
        if (dimension < 2)
            throw IllegalDimensionException(dimension);
        if (maxTime < 1) {
            throw IllegalMaxTimeException(maxTime);
        }

        std::vector<Halfspace<Number>> box;

        for (int i = 0; i < dimension; i++) {
            vector_t<Number> direction = vector_t<Number>::Zero(dimension);
            direction[i] = -1;
  
            Halfspace<Number> lowerHsp(direction,0);
            box.emplace_back(lowerHsp);

            Halfspace<Number> upperHsp(-direction,maxTime);
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

    Halfspace<Number> STDiagram::createHalfspaceFromEvent(const Event &event, bool isSourceEvent) {
        vector_t<Number> direction = vector_t<Number>::Zero(event.getGeneralDependencies().size()+1);

        int i = 0;
        for (; i < event.getGeneralDependencies().size(); i++) {
            direction[i] = event.getGeneralDependencies()[i];
        }

        if (isSourceEvent) {
            direction[i] = -1;
        } else {
             direction[i] = 1;
        }

        Halfspace<Number> hsp(direction,event.getTime());
        return hsp;
    }

    bool STDiagram::isValidEvent(const Event &event) {
        if (!(event.getGeneralDependencies().size() > 0)) { 
            return false;
        }
        return true;
    }

    void STDiagram::print(const vector<Region> &regionsToPrint, bool cummulative) {
        Plotter<Number>& plt = Plotter<Number>::getInstance();
        plt.rSettings().cummulative = cummulative;

        for (Region region : regionsToPrint) {
            plt.addObject(region.vertices());
        }

        plt.plot2d();
    }

}