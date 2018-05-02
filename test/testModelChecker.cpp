#include "gtest/gtest.h"
#include "ModelChecker.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"
#include "STDiagram.h"
#include "ParametricLocationTree.h"

using namespace std;
using namespace hpnmg;

#define MAX_TIME 10

class ModelCheckerTest : public ::testing::Test {
protected:
    ParametricLocationTree parametricLocationTree;

    ModelCheckerTest() : parametricLocationTree(
            ParametricLocation(vector<int>{1, 1}, vector<vector<double>>{vector<double>{0, 0}}, vector<double>{1}, 1,
                               Event(EventType::Root, vector<double>{0}, 0), vector<vector<vector<double>>> {{{0}}},
                               vector<vector<vector<double>>> {{{10}}}), 10) {

    }

    virtual void SetUp() {

        ParametricLocationTree::Node rootNode = parametricLocationTree.getRootNode();
        
        ParametricLocation childNode = rootNode.getParametricLocation();

        ParametricLocationTree::Node detNode = parametricLocationTree.setChildNode(rootNode,
             ParametricLocation(vector<int>{1, 0}, vector<vector<double>>{vector<double>{0, 5}}, vector<double>{2},
                                1, Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{5}}},
                                vector<vector<vector<double>>> {{{10}}}));

        ParametricLocationTree::Node stocNode = parametricLocationTree.setChildNode(rootNode, ParametricLocation(
                vector<int>{0, 1}, vector<vector<double>>{vector<double>{1, 0}}, vector<double>{-1}, 1,
                Event(EventType::General, vector<double>{1}, 0), vector<vector<vector<double>>> {{{0}}},
                vector<vector<vector<double>>> {{{5}}}));

        ParametricLocationTree::Node emptyNode = parametricLocationTree.setChildNode(stocNode, ParametricLocation(
                vector<int>{0, 1}, vector<vector<double>>{vector<double>{2, 0}}, vector<double>{0}, 1,
                Event(EventType::General, vector<double>{2}, 0), vector<vector<vector<double>>> {{{0}}},
                vector<vector<vector<double>>> {{{2.5}}}));

        ParametricLocationTree::Node timedAfterStochastic = parametricLocationTree.setChildNode(stocNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{2, -5}}, vector<double>{0}, 1,
                                 Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{2.5}}},
                                 vector<vector<vector<double>>> {{{5}}}));

        ParametricLocationTree::Node timedAfterEmpty = parametricLocationTree.setChildNode(emptyNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{0, 0}}, vector<double>{0}, 1,
                                 Event(EventType::Timed, vector<double>{0}, 5), vector<vector<vector<double>>> {{{0}}},
                                 vector<vector<vector<double>>> {{{2.5}}}));

        ParametricLocationTree::Node stochasticAfterTimed = parametricLocationTree.setChildNode(detNode,
              ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{2, -5}}, vector<double>{0}, 1,
                                 Event(EventType::General, vector<double>{1}, 0), vector<vector<vector<double>>> {{{5}}},
                                 vector<vector<vector<double>>> {{{7.5}}}));

        ParametricLocationTree::Node fullNode = parametricLocationTree.setChildNode(detNode, ParametricLocation(
                vector<int>{1, 0}, vector<vector<double>>{vector<double>{0, 10}}, vector<double>{0}, 1,
                Event(EventType::Timed, vector<double>{0}, 7.5), vector<vector<vector<double>>> {{{7.5}}},
                vector<vector<vector<double>>> {{{10}}}));

        ParametricLocationTree::Node stochasticAfterFull = parametricLocationTree.setChildNode(fullNode,
               ParametricLocation(vector<int>{0, 0}, vector<vector<double>>{vector<double>{0, 10}}, vector<double>{0}, 1,
                                  Event(EventType::Timed, vector<double>{1}, 0), vector<vector<vector<double>>> {{{7.5}}},
                                  vector<vector<vector<double>>> {{{10}}}));
    }

    virtual void TearDown() {

    }
};

TEST_F(ModelCheckerTest, DiscreteFormulaTest) {
    parametricLocationTree.updateRegions();
    vector<carl::Interval<double>> oneIntervalSet = ModelChecker::discreteFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 1, 3);

    ASSERT_EQ(oneIntervalSet.size(), 1);
    ASSERT_EQ(oneIntervalSet[0].lower(), 3);
    ASSERT_EQ(oneIntervalSet[0].upper(), 10);

    vector<carl::Interval<double>> threeIntervalSets = ModelChecker::discreteFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 0, 6);

    ASSERT_EQ(threeIntervalSets.size(), 3);

    sort(threeIntervalSets.begin(), threeIntervalSets.end(), ModelChecker::sortIntervals);

    ASSERT_EQ(threeIntervalSets[0].lower(), 0);
    ASSERT_EQ(threeIntervalSets[0].upper(), 2.5);
    ASSERT_EQ(threeIntervalSets[1].lower(), 2.5);
    ASSERT_EQ(threeIntervalSets[1].upper(), 5);
    ASSERT_EQ(threeIntervalSets[2].lower(), 5);
    ASSERT_EQ(threeIntervalSets[2].upper(), 6);
}


TEST_F(ModelCheckerTest, ContinuousFormulaTest) {
    parametricLocationTree.updateRegions();
    vector<carl::Interval<double>> intervalSets = ModelChecker::continuousFormulaIntervalSetsAtTime(
            parametricLocationTree, 0, 4, 3);

    ASSERT_EQ(intervalSets.size(), 3);

    sort(intervalSets.begin(), intervalSets.end(), ModelChecker::sortIntervals);

    ASSERT_EQ(intervalSets[0].lower(), 0);
    ASSERT_EQ(intervalSets[0].upper(), 1.5);
    ASSERT_EQ(intervalSets[1].lower(), 1.5);
    ASSERT_EQ(intervalSets[1].upper(), 3);
    ASSERT_EQ(intervalSets[2].lower(), 3);
    ASSERT_EQ(intervalSets[2].upper(), 10);
}

class ModelCheckerTestXML : public ::testing::Test {
protected:

    shared_ptr<hpnmg::ParametricLocationTree> plt;

    ModelCheckerTestXML() {
        ReadHybridPetrinet reader;
        //TODO Check impl. Pointer seems to be a problem here. (Memory Leak?)
        shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
        ParseHybridPetrinet parser;
        shared_ptr<hpnmg::ParametricLocationTree> pltC = parser.parseHybridPetrinet(hybridPetrinet, 20);
        plt = pltC;
    }
};

TEST_F(ModelCheckerTestXML, DiscreteFormulaTest) {
    plt->updateRegions();
    std::vector<Region> regions = ModelChecker::discreteFormulaRegionsAtTime(plt, 0, 0, 2, true);
    //STDiagram::print(regions, true);
    //dsprintf("Number of Regions %lu.\n", regions.size());
}

TEST_F(ModelCheckerTestXML, FormulaTest) {
    plt->updateRegions();
    std::vector<Region> regions;
    double time = 5;
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(time);
    for(ParametricLocationTree::Node candidateNode : candidates) {
        Region r  = ModelChecker::dfml(candidateNode, 0, 0, false);
        if (STDiagram::regionIsCandidateForTime(time,r,candidateNode.getParametricLocation().getDimension()).first == true)
            regions.insert(regions.end(), candidateNode.getRegion());
    }
    //STDiagram::print(regions, true);
}

TEST_F(ModelCheckerTestXML, SimpleNegationFormulaTest) {
    plt->updateRegions();
    std::vector<Region> regions;
    std::vector<Region> not_regions;
    double time = 3;
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(time);
    for(ParametricLocationTree::Node candidateNode : candidates) {
        Region r  = ModelChecker::cfml(candidateNode, 0, 3, false);
        regions.push_back(r);

        std::vector<Region> phi_1 = ModelChecker::neg({r}, candidateNode.getRegion());       
        if (phi_1.size()>0) {
            not_regions.insert(not_regions.end(), phi_1.begin(), phi_1.end());
        }
                
    }

    //TODO Write Tests

    //STDiagram::print(regions, false, "phi");
    //STDiagram::print(not_regions, false, "not_phi");
    
}

TEST_F(ModelCheckerTestXML, KomplexNegationFormulaTest) {
    plt->updateRegions();
    std::vector<Region> regions;
    std::vector<Region> not_regions;
    double time = 3;
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(time);
    for(ParametricLocationTree::Node candidateNode : candidates) {
        Region r  = ModelChecker::cfml(candidateNode, 0, 3, false);
        std::vector<Region> phi_1 = ModelChecker::neg({r}, candidateNode.getRegion());

        r = ModelChecker::cfml(candidateNode, 0, 4, false);

        std::vector<Region> con = ModelChecker::conj(phi_1, {r});
        if (con.size()>0) {
            regions.insert(regions.end(), con.begin(), con.end());
        }

        std::vector<Region> not_con = ModelChecker::neg(con, candidateNode.getRegion());
        if (not_con.size()>0) {
            not_regions.insert(not_regions.end(), not_con.begin(),not_con.end());
        }
                
    }
    
    //TODO Write Tests

    //STDiagram::print(regions, false, "phi");
    //STDiagram::print(not_regions, false, "not_phi");
}

void printIntervals(Intervals intervals) {
    for (carl::Interval<double> i: intervals) {
        printf("[%f,%f] ", i.lower(), i.upper());
    }
}

void printIntervals(std::vector<Intervals> intervals) {
    for (Intervals I : intervals) {
        printf("{ ");
        printIntervals(I);
        printf("}");
    }
    printf("\n");
}

std::vector<Intervals> getIntervals(std::vector<Region> rs, double time) {
    std::vector<Intervals> IS;
    for(Region r : rs){
        if (!r.empty()) {
            Intervals br = STDiagram::createIntervals(r);
            IS.push_back(br);
        }
    } 
    return IS;
}

std::vector<Intervals> getIntervals(std::vector<Region> rs, double time, Halfspace<double> hsp) {
    std::vector<Intervals> IS;
    for(Region r : rs){
        if (!r.empty()) {
            r.insert(hsp);
            if (!r.empty()) {
                Intervals br = STDiagram::createIntervals(r);
                IS.push_back(br);
            }
        }       
        
    } 
    return IS;
}

std::vector<Region> until(shared_ptr<hpnmg::ParametricLocationTree> plt, ParametricLocationTree::Node node, double time) {
    printf("---Model Checking Until---\n");
    std::pair<bool, Region> res = STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(0,time), node.getRegion(), node.getParametricLocation().getDimension());
    if (!res.first) {
        return {};
    }

    //This should be replaced by parsing given formulas (Given from parameters of the function - we need a parser for this)
    std::vector<Region> sat1 = {ModelChecker::cfml(node, 0, 3, false)};
    std::vector<Region> sat2 = ModelChecker::neg({sat1}, node.getRegion());

    Halfspace<double> sourceHalfspace = STDiagram::createHalfspaceFromEvent(STDiagram::makeValidEvent(node.getParametricLocation().getSourceEvent(), node.getRegion()), false);

    //printf("Number of regions satisfying phi2: %lu\n", sat2.size());
    std::vector<Intervals> I1 = getIntervals(sat2, time, sourceHalfspace);
    printf("I1: ");
    printIntervals(I1);
    
    //printf("Number of regions satisfying phi1: %lu\n", sat1.size());
    std::vector<Intervals> iCandidates = getIntervals(sat1, time, sourceHalfspace);
    printf("Candidates: ");
    printIntervals(iCandidates);

    std::vector<Region> satPhi1AndPhi2 = ModelChecker::conj(sat1,sat2);
    //printf("Number of regions satisfying Phi1AndPhi2: %lu\n", satPhi1AndPhi2.size());
    std::vector<Intervals> iPhi1AndPhi2 = getIntervals(satPhi1AndPhi2, time);
    printf("Phi1AndPhi2: ");
    printIntervals(iPhi1AndPhi2);

    std::vector<Region> satNotPhi1 = ModelChecker::neg(sat1, node.getRegion());
    std::vector<Region> satNotPhi2 = ModelChecker::neg(sat2, node.getRegion());
    std::vector<Region> satPhi1AndNotPhi2 = ModelChecker::conj(sat1,satNotPhi2);

    std::vector<Region> satFacets = ModelChecker::conj(satPhi1AndNotPhi2, satNotPhi1);
    // Remove all facets that are the border to Sat(Phi2)
    std::vector<Region> nonEmpty;
    for (Region r : satFacets) {
        for (Region s2 : sat2) {
            if (s2.intersect(r).empty()) {
                nonEmpty.push_back(r);
            }
        }
    }
    STDiagram::print(nonEmpty, false, "nonEmpty");
    satFacets = nonEmpty;
    STDiagram::print(satFacets, false, "satFacets");
    //printf("Number of Facet regions: %lu\n", satFacets.size());
    std::vector<Intervals> iFacets = getIntervals(satFacets, time); 
    printf("Facets: ");
    printIntervals(iFacets);

    std::vector<Intervals> hI2 = STDiagram::differenceOfIntervals(iCandidates, iFacets);
    printf("hI2:");
    printIntervals(hI2);
    std::vector<Intervals> I2 = STDiagram::intersectionOfIntervals(hI2,iPhi1AndPhi2); 
    printf("I2: ");
    printIntervals(I2);

    printIntervals(iCandidates);
    std::vector<Intervals> rest = STDiagram::differenceOfIntervals(iCandidates, I1);
    rest = STDiagram::differenceOfIntervals(rest, I2);
    printf("Rest: ");
    printIntervals(rest);

    std::vector<Intervals> I3;
    if (rest.size()>0) {
        for (ParametricLocationTree::Node c : plt->getChildNodes(node)) {
            ParametricLocationTree::Node r(c);
            Region restRegion = STDiagram::boundRegionByIntervals(c.getRegion(),rest,time)[0];
            if(!restRegion.empty()) {
                r.setRegion(restRegion);
            }            
            std::vector<Region> innerRegions = until(plt, r, time);
            if (innerRegions.size() > 0) {
                std::vector<Intervals> inner =  STDiagram::intervalsFromRegions(innerRegions);
                I3.insert(I3.end(), inner.begin(), inner.end());
            }            
        }
    }
    printf("I3: ");
    printIntervals(I3);

    std::vector<Intervals> result = STDiagram::unionOfIntervals(I1,I2);
    result = STDiagram::unionOfIntervals(result, I3);
    printf("Result of Until Model Checking: ");
    printIntervals(result);
    return STDiagram::boundRegionByIntervals(node.getRegion(), result, time);   
}

TEST_F(ModelCheckerTestXML, UntilFormulaTest) {
    plt->updateRegions();
    std::vector<Region> regions;
    double time = 4;
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTimeInterval(std::pair<double,double>(0,3));

    for (ParametricLocationTree::Node candidate : candidates) {
        std::vector<Region> sat = until(plt, candidate, time);
        regions.insert(regions.end(), sat.begin(), sat.end());
    }
    
    
    /*for(ParametricLocationTree::Node candidateNode : candidates) {
                
    }*/
    
    //TODO Write Tests

    STDiagram::print(regions, false, "until");
    //STDiagram::print(not_regions, false, "not_phi");
}
