#pragma once

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

#include "ParametricLocationTree.h"


namespace hpnmg {

    class PLTWriter {
    private:
        xercesc::DOMImplementation *domImplementation;
        xercesc::DOMLSSerializer *serializer;
        xercesc::XMLFormatTarget *target;

    public:
        PLTWriter();

        void writePLT(shared_ptr<ParametricLocationTree> plt, double maxTime);

        xercesc::DOMNode *
        generateParametricLocationElement(shared_ptr<ParametricLocationTree> plt, ParametricLocationTree::Node node,
                                          xercesc::DOMDocument *domNode);
        std::string getEventName(EventType eventType);
    };
}
