#pragma once

#include <string>

using namespace std;
namespace hpnmg {

    class AleatoryVariable {
    private:
        string generalTransitionID;
        unsigned long firingNumber;
    public:
        AleatoryVariable(string &generalTransitionID, unsigned long firingNumber);
        string getGeneralTransitionID();
        unsigned long getFiringNumber();
    };
}
