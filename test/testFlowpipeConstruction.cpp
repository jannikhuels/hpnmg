#include <hpnmg/flowpipeConstruction/ReachRHPn.h>
#include "gtest/gtest.h"
#include "flowpipeConstruction/ReachRHPn.h"
#include "ReadHybridPetrinet.h"
#include "datastructures/HybridPetrinet.h"

using namespace hpnmg;

TEST(FlowpipeConstruction, Basic)
{

    typedef mpq_class Number;
    typedef hypro::HPolytope<Number> Representation;

    // setup
    string filePath = "../../test/testfiles/RHPn/simplerhpn.xml";
    double tauMax = 10.0;

    // read
    ReadHybridPetrinet reader;
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);


    //hypro::HybridAutomaton<Number> mAutomaton;
    hypro::ReachabilitySettings settings;
    ReachRHPn<Number,ReachRHPnSettings,hypro::State_t<Number>> reacher(*hybridPetriNet,settings);

    settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(3);
    settings.jumpDepth = 3;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    reacher.initQueue();

    /*// perform reachability analysis.
    auto flowpipeIndices = reacher.computeForwardReachability();

    // PLOTTING
    // plot flowpipes.
    Plotter<Number>& plotter = Plotter<Number>::getInstance();
    plotter.setFilename("simplerhpn");
    for(auto& indexPair : flowpipeIndices){
        std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
        // Plot single flowpipe
        for(auto& set : flowpipe) {
            std::vector<Point<Number>> points = set.vertices();
            if(!points.empty() && points.size() > 2) {
                for(auto& point : points) {
                    std::cout << point << endl;
                    point.reduceDimension(2);
                }
                plotter.addObject(points);
                points.clear();
            }
        }
    }
    // write output.
    plotter.plotTex();*/

    //reacher.computeForwardReachability();

    /*hypro::WorkQueue<TaskTypePtr>& mWorkingQueue = reacher.rGetQueue();

    // collect all computed reachable states
    std::vector<std::pair<unsigned, typename ReachRHPn<Number,ReachRHPnSettings,State>::flowpipe_t>> collectedReachableStates;

    std::cout << std::endl << "Queue size: " << mWorkingQueue.size() << std::flush;
    std::cout << std::endl;

    while ( !mWorkingQueue.isEmpty() ) {
    TaskTypePtr nextInitialSet = mWorkingQueue.dequeueFront();


    std::cout << "\rQueue size: " << mWorkingQueue.size() << std::flush;


    mCurrentLevel = nextInitialSet->first;
    std::cout <<"Depth " << mCurrentLevel << ", Location: " << nextInitialSet->second.getLocation()->getName() << std::endl;
    assert(int(mCurrentLevel) <= mSettings.jumpDepth);
    TRACE("hypro.reacher","Obtained set of type " << nextInitialSet->second.getSetType() << ", requested type is " << mType);
    flowpipe_t newFlowpipe = computeForwardTimeClosure(nextInitialSet->second);

    collectedReachableStates.emplace_back(std::make_pair(nextInitialSet->second.getLocation()->hash(), newFlowpipe));
    }

    return collectedReachableStates;*/

}
