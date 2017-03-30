#pragma once

#include "GeneralExceptions.h"

using namespace std;

namespace hpnmg {
    class IllegalDimensionException: public invalid_argument, public Exception {
    public:
        IllegalDimensionException(int dimension): invalid_argument("IllegalDimensionException:") {
            ostringstream ss;
            ss << invalid_argument::what() << ": Dimension must be >= 2 but it is ( " << dimension << " ).";
            errorDescription.append(ss.str());
        }

        virtual const char* what() const throw()
        {   
            return errorDescription.c_str();
        }
    };

    class IllegalMaxTimeException: public invalid_argument, public Exception {
    public:
        IllegalMaxTimeException(int maxTime): invalid_argument("IllegalMaxTimeException:") {
            ostringstream ss;
            ss.str("");
            ss << invalid_argument::what() << ": MaxTime must be >= 1 but it is ( " << maxTime << " ).";
            errorDescription.append(ss.str());
        }

        virtual const char* what() const throw()
        {   
            
            return errorDescription.c_str();
        }
    };
}