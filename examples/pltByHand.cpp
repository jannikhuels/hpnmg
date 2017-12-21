#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ParametricLocationTree.h"
#include "ModelChecker.h"
#include "representations/GeometricObject.h"
#include "util/Plotter.h"
#include <gmpxx.h>

#define maxtime 10

using namespace hpnmg;

ParametricLocationTree plt(ParametricLocation(std::vector<int>{1,1},std::vector<std::vector<double>>{std::vector<double>{0,0}},std::vector<double>{2},2,Event(EventType::Root, vector<double>{0,0}, 0), std::vector<double> {0,0}, std::vector<double> {maxtime,maxtime}), maxtime);

void setupTree() {
  cout << "Dimensions: " << plt.getDimension() << endl;

  ParametricLocationTree::Node i = plt.getRootNode();
  // TODO: Intervals must be dependent on s
  // Region 1
  ParametricLocationTree::Node ii = plt.setChildNode(i, 
    ParametricLocation(std::vector<int>{0,1},
    std::vector<std::vector<double>>{std::vector<double>{2,0,0}},
    std::vector<double>{-1},
    2,
    Event(EventType::General, vector<double>{1,0}, 0), std::vector<double> {0,0}, std::vector<double> {5,5}));

  ParametricLocationTree::Node iii = plt.setChildNode(i, 
    ParametricLocation(std::vector<int>{1,0},
    std::vector<std::vector<double>>{std::vector<double>{0,2,0}},
    std::vector<double>{3},
    2,
    Event(EventType::General, vector<double>{0,1}, 0), std::vector<double> {0,0}, std::vector<double> {5,5}));

  ParametricLocationTree::Node iv = plt.setChildNode(i, 
    ParametricLocation(std::vector<int>{1,1},
    std::vector<std::vector<double>>{std::vector<double>{0,0,10}},
    std::vector<double>{0},
    2,
    Event(EventType::General, vector<double>{0,1}, 0), std::vector<double> {5,5}, std::vector<double> {maxtime,maxtime}));


  // Region 2
  ParametricLocationTree::Node v = plt.setChildNode(ii, 
    ParametricLocation(std::vector<int>{0,0},
    std::vector<std::vector<double>>{std::vector<double>{3,-1,0}},
    std::vector<double>{0},
    2,
    Event(EventType::General, vector<double>{0,1}, 0), std::vector<double> {0,0}, std::vector<double> {5,maxtime}));
}

void printIntervals(std::vector<carl::Interval<double>> intervalSets) {
  for (carl::Interval<double> interval : intervalSets) {
    cout << "[" << interval.lower() << ", " << interval.upper() << "],";
  }
  cout << endl;
}
 
int main (int argc, char *argv[])
{
  setupTree();

  plt.updateRegions();

  std::vector<carl::Interval<double>> intervalSets = ModelChecker::discreteFormulaIntervalSetsAtTime(plt, 0, 1, 3);
  std::sort(intervalSets.begin(), intervalSets.end(), ModelChecker::sortIntervals);
  printIntervals(intervalSets);
  
  return 0;
}