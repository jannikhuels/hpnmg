#pragma once

#include "GeneralExceptions.h"


namespace hpnmg {
    using namespace std;
    class NoSuchNodeException: public runtime_error, public Exception {
    public:
        NoSuchNodeException(int id): runtime_error("NoSuchNodeException:") {
            ostringstream ss;
            ss << runtime_error::what() << ": No node with parent node id: ( " << id << " ) found.";
            errorDescription.append(ss.str());
        }

        virtual const char* what() const throw()
        {   
            return errorDescription.c_str();
        }
    };

    class RootNodeExistsException : public runtime_error, public Exception {
    public:
        RootNodeExistsException() : runtime_error("RootNodeExistsException:") {
            ostringstream ss;
            ss << runtime_error::what() << ": Root node already existing.";
            errorDescription.append(ss.str());
        }

        virtual const char* what() const throw()
        {   
            return errorDescription.c_str();
        }
    };

    class InvalidParentNodeException : public invalid_argument, public Exception {
    public:
        InvalidParentNodeException() : invalid_argument("InvalidParentNodeException:") {
            ostringstream ss;
            ss << invalid_argument::what() << ": The given parent node is invalid.";
            errorDescription.append(ss.str());
        }

        virtual const char* what() const throw()
        {   
            return errorDescription.c_str();
        }
    };
}