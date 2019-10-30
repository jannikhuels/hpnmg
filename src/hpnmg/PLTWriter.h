#pragma once

#include "ParametricLocationTree.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

namespace hpnmg {

    class PLTWriter {
    private:
        xercesc::DOMImplementation *domImplementation;
        xercesc::DOMLSSerializer *serializer;
        xercesc::XMLFormatTarget *target;

    public:
        PLTWriter();

        void writePLT(shared_ptr<ParametricLocationTree> plt, double maxTime);

        void writePLT(shared_ptr<ParametricLocationTree> plt, double maxTime, string filepath);

        xercesc::DOMNode *
        generateParametricLocationElement(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node node,
                                          xercesc::DOMDocument *domNode);
        std::string getEventName(EventType eventType);
        std::string formatTimes(double time, vector<double> generalDependencies);
        std::string formatTimes(vector<double> generalDependencies);
    };
}
