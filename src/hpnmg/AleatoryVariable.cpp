#include "AleatoryVariable.h"

using namespace std;
namespace hpnmg {
    AleatoryVariable::AleatoryVariable(string &generalTransitionID, unsigned long firingNumber) :
            generalTransitionID(generalTransitionID), firingNumber(firingNumber) {}

    string AleatoryVariable::getGeneralTransitionID() { return generalTransitionID; }

    unsigned long AleatoryVariable::getFiringNumber() { return firingNumber; }
}