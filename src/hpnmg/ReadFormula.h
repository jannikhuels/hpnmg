#ifndef HPNMG_READFORMULA_H
#define HPNMG_READFORMULA_H

#include "modelchecker/Formula.h"
#include "modelchecker/Conjunction.h"
#include "modelchecker/Negation.h"
#include "modelchecker/Until.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <memory>
#include <string>
#include <utility>

namespace hpnmg {
    class ReadFormula {
    private:
        xercesc::XercesDOMParser *xmlParser;
        std::shared_ptr<Formula> formula;

        bool validateSchema(const std::string &formulaFilePath);
        Formula parseFormula(xercesc::DOMElement *formulaNode);
        std::pair<std::string, double> atomicAttributes(xercesc::DOMElement *formulaNode);

        // With more than on child element with a specific name the count parameter may be used.
        // The first occurence of an element has count 1.
        xercesc::DOMElement* nextChildElement(xercesc::DOMElement *formulaNode, std::string tagName, int count = 1);

    public:
        ReadFormula();
        ~ReadFormula();

        Formula readFormula(const std::string &formulaFilePath);
    };
}



#endif //HPNMG_READFORMULA_H
