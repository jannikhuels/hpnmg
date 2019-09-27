#include "gtest/gtest.h"
#include "flowpipeConstruction/ReachRHPn.h"
#include "ReadHybridPetrinet.h"
#include "HybridPetrinet.h"

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


    hypro::HybridAutomaton<Number> mAutomaton;
    ReachRHPn<Number,ReachRHPnSettings,hypro::State_t<Number>> reacher(mAutomaton);

    hypro::ReachabilitySettings settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(3);
    settings.jumpDepth = 3;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

}
