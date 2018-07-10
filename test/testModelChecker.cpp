#include "gtest/gtest.h"
#include "ModelChecker.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"
#include "STDiagram.h"
#include "ParametricLocationTree.h"
#include <chrono>

using namespace std;
using namespace hpnmg;

#define MAX_TIME 10

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

class ModelCheckerTestComplexXML : public ::testing::Test {
protected:

    shared_ptr<hpnmg::ParametricLocationTree> plt;

    ModelCheckerTestComplexXML() {
        ReadHybridPetrinet reader;
        //TODO Check impl. Pointer seems to be a problem here. (Memory Leak?)
        shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("jannik1general.xml");
        ParseHybridPetrinet parser;
        int maxTime = 20;
        shared_ptr<hpnmg::ParametricLocationTree> pltC = parser.parseHybridPetrinet(hybridPetrinet, maxTime);
        plt = pltC;
        
    }
};

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

std::vector<Intervals> getIntervals(std::vector<Region> rs) {
    std::vector<Intervals> IS;
    for(Region r : rs){
        if (!r.empty()) {
            Intervals br = STDiagram::createIntervals(r);
            if (std::find(IS.begin(), IS.end(), br) == IS.end()) {
                IS.push_back(br);
            }
            
        }
    } 
    return IS;
}

std::vector<Intervals> getIntervals(std::vector<Region> rs, Halfspace<double> hsp) {
    std::vector<Intervals> IS;
    for(Region r : rs){
        if (!r.empty()) {
            r.insert(hsp);
            if (!r.empty()) {
                Intervals br = STDiagram::createIntervals(r);
                if (std::find(IS.begin(), IS.end(), br) == IS.end()) {
                    IS.push_back(br);
                }
            }
        }       
        
    } 
    return IS;
}

std::vector<Region> until(shared_ptr<hpnmg::ParametricLocationTree> plt, ParametricLocationTree::Node node, Halfspace<double> hUpperTime, Halfspace<double> hLowerTime) {
    printf("---Model Checking Until---\n");
    Region region = node.getRegion();
    region.insert(hUpperTime);
    if (region.empty()) {
        return {};
    }

    //This should be replaced by parsing given formulas (Given from parameters of the function - we need a parser for this)
    std::vector<Region> sat1 = {ModelChecker::dfml(node, 0, 1, false)};
    std::vector<Region> sat2 = {ModelChecker::cfml(node,0,0,false)};

    Halfspace<double> sourceHalfspace = hLowerTime;

    //printf("Number of regions satisfying phi2: %lu\n", sat2.size());

    //TODO GET Seems not to be uncorrect for child regions? 
    std::vector<Intervals> I1 = getIntervals(sat2, sourceHalfspace);
    //printf("I1: ");
    //printIntervals(I1);
    
    //printf("Number of regions satisfying phi1: %lu\n", sat1.size());
    std::vector<Intervals> iCandidates = getIntervals(sat1, sourceHalfspace);
    //cout << "Number of candidates: " << iCandidates.size() << endl;
    //printf("Candidates: ");
    //printIntervals(iCandidates);

    if (iCandidates.size() == 0) {
        return {};
    }

    std::vector<Region> satPhi1AndPhi2 = ModelChecker::conj(sat1,sat2);
   //printf("Number of regions satisfying Phi1AndPhi2: %lu\n", satPhi1AndPhi2.size());
    std::vector<Intervals> iPhi1AndPhi2 = getIntervals(satPhi1AndPhi2);
    //printf("Phi1AndPhi2: ");
    //printIntervals(iPhi1AndPhi2);

    std::vector<Region> satNotPhi1 = ModelChecker::neg(sat1, node.getRegion());
    std::vector<Region> satNotPhi2 = ModelChecker::neg(sat2, node.getRegion());
    std::vector<Region> satPhi1AndNotPhi2 = ModelChecker::conj(sat1,satNotPhi2);

    std::vector<Region> satFacets = ModelChecker::conj(satPhi1AndNotPhi2, satNotPhi1);
    //cout << "Number of facets " << satFacets.size() << endl;
    // Remove all facets that are the border to Sat(Phi2)
    std::vector<Region> nonEmpty;
    for (Region r : satFacets) {
        for (Region s2 : sat2) {
            if (s2.intersect(r).empty()) {
                nonEmpty.push_back(r);
            }
        }
    }
    //STDiagram::print(nonEmpty, false, "nonEmpty");
    satFacets = nonEmpty;
    //STDiagram::print(satFacets, false, "satFacets");
    //printf("Number of Facet regions: %lu\n", satFacets.size());
    std::vector<Intervals> iFacets = getIntervals(satFacets); 
    //printf("Facets: ");
    //printIntervals(iFacets);

    std::vector<Intervals> hI2 = STDiagram::differenceOfIntervals(iCandidates, iFacets);
    //printf("hI2:");
    //printIntervals(hI2);
    std::vector<Intervals> I2 = STDiagram::intersectionOfIntervals(hI2,iPhi1AndPhi2); 


    //printIntervals(iCandidates);
    std::vector<Intervals> rest = STDiagram::differenceOfIntervals(iCandidates, I1);
    rest = STDiagram::differenceOfIntervals(rest, I2);
    printf("Rest: ");
    printIntervals(rest);

    std::vector<Intervals> I3;
    //cout << "Size of rest " << rest.size() << endl;
    if (rest.size()>0) {
        for (ParametricLocationTree::Node c : plt->getChildNodes(node)) {
            Halfspace<double> hChild = STDiagram::createHalfspaceFromEvent(STDiagram::makeValidEvent(c.getParametricLocation().getSourceEvent(), c.getRegion()), true);      
            std::vector<Region> innerRegions = until(plt, c, hUpperTime, hChild);
            if (innerRegions.size() > 0) {
                std::vector<Intervals> inner =  STDiagram::intervalsFromRegions(innerRegions);
                STDiagram::intersectionOfIntervals(inner, rest);
                I3 = STDiagram::unionOfIntervals(I3, inner);
            }            
        }
    }
    //printf("I3: ");
    //printIntervals(I3);
    cout << "I1" << endl;
    printIntervals(I1);
    cout << "I2" << endl;
    printIntervals(I2);
    cout << "I3" << endl;
    printIntervals(I3);
    cout << "-----------------" << endl;
    std::vector<Intervals> result = STDiagram::unionOfIntervals(I1,I2);
    result = STDiagram::unionOfIntervals(result, I3);
    result = STDiagram::removeEmptyIntervals(result);
    printf("Result of Until Model Checking: ");
    printIntervals(result);
    return STDiagram::boundRegionByIntervals(node.getRegion(), plt->getMaxTime(), result, hUpperTime);   
}

TEST_F(ModelCheckerTestComplexXML, UntilFormulaTest) {

    
    int dim = plt->getDimension();

    cout << "Dimension is " << dim << endl; 

    plt->updateRegions();
    std::vector<ParametricLocationTree::Node> allRegions = plt->getCandidateLocationsForTimeInterval(std::pair<double,double>(0,20));
    cout << "Number of Regions is " << allRegions.size() << endl;    
    
    std::vector<Region> regions;
    double time = 0;
    double timeT = 4 + time;
    double cfmlTime = 4;
    
    //std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(0);
    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(cfmlTime);
        

    Halfspace<double> upperTimeHsp = STDiagram::createHalfspaceForTime(timeT, dim);
    Halfspace<double> startTimeHsp = STDiagram::createHalfspaceForTime(time, dim);

    cout << "Start Model Checking." << endl;
    cout << candidates.size()  << " candidates found" << endl;
    auto startTime = chrono::high_resolution_clock::now();
    for (ParametricLocationTree::Node candidate : candidates) {
        //std::vector<Region> sat = until(plt, candidate, upperTimeHsp, startTimeHsp);
        Region sat = ModelChecker::dfml(candidate,0,0,false);
        sat = STDiagram::createTimeRegion(sat, cfmlTime, dim);
        //std::vector<Region> satNeg = ModelChecker::neg({sat}, candidate.getRegion());
        /*if (sat.size() > 0) {
            regions.insert(regions.end(), satNeg.begin(), satNeg.end());
        } */  
        if (!sat.empty()) {
            regions.push_back(sat);
        }    
    }
    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = endTime - startTime;
    
    
    //TODO Write Tests
    cout << "Number of regions that satisfy until: " << regions.size() << endl;
    cout << "It took " << elapsed.count() << "ms" << endl;

    cout << "Satisfaction intervals of returned regions:" << endl;
    std::vector<Intervals> sat =  STDiagram::intervalsFromRegions(regions);
    sat = STDiagram::removeEmptyIntervals(sat);
    
    printIntervals(sat);
    cout << "Number of satisfaction intervals: " << sat.size() << endl;

    //std::vector<Region> printRegions;
    //STDiagram::print(regions, true, "candidates4Until");

    //STDiagram::print(regions, false, "until");
    //STDiagram::print(not_regions, false, "not_phi");
}
