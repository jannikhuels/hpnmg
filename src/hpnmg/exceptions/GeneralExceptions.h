#pragma once

#include <exception>
#include <sstream>
#include <string>

using namespace std;

namespace hpnmg {
    class Exception {
    protected:
        string errorDescription;
    public:
        Exception() : errorDescription("") {}
    };

    class IllegalArgumentException: public invalid_argument , public Exception{
    public:
        IllegalArgumentException(string argumentName, string reason = "") : invalid_argument("IllegalArgumentException:") {
            ostringstream ss;
            ss.str("");
            ss << invalid_argument::what() << " Argument with name " << argumentName << " is invalid. " << reason;
            errorDescription.append(ss.str());
        } 

        virtual const char* what() const throw()
        {   
            return errorDescription.c_str();
        }
    };
}