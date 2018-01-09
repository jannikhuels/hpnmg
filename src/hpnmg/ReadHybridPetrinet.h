#pragma once

#include <memory>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>


#include "datastructures/HybridPetriNet.h"
#include "datastructures/places/DiscretePlace.h"
#include "datastructures/places/FluidPlace.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>


namespace hpnmg {

    class ReadHybridPetrinet {

    private:
        xercesc::XercesDOMParser *xmlParser;
        HybridPetriNet hybridPetriNet;

    public:
        ReadHybridPetrinet();
        ~ReadHybridPetrinet();

        HybridPetriNet* readHybridPetrinet(const std::string &filepath);
        void parsePlaces(xercesc::DOMElement* placesNode);

    };

}
