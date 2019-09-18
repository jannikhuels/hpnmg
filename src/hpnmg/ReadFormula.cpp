#include "ReadFormula.h"

using namespace xercesc;
using namespace std;

namespace hpnmg {
    ReadFormula::ReadFormula() {
        try {
            XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
        }
        catch (XMLException &e) {
            char *message = XMLString::transcode(e.getMessage());
            cerr << "XML toolkit initialization error: " << message << endl;
            XMLString::release(&message);
            // throw exception here to return ERROR_XERCES_INIT
        }

        // Can't call transcode till after Xerces Initialize()
        xmlParser = new XercesDOMParser;
    }

    ReadFormula::~ReadFormula() {
        // Free memory

        delete xmlParser;

        // Terminate Xerces
        try {
            XMLPlatformUtils::Terminate();  // Terminate after release of memory
        }
        catch (xercesc::XMLException &e) {
            char *message = xercesc::XMLString::transcode(e.getMessage());

            cerr << "XML toolkit teardown error: " << message << endl;
            XMLString::release(&message);
        }
    }

    // src: https://sivachandranp.wordpress.com/2010/10/10/xml-schema-validation-using-xerces-c/
    class ParserErrorHandler : public ErrorHandler
    {
    private:
        void reportParseException(const SAXParseException& ex)
        {
            char* msg = XMLString::transcode(ex.getMessage());
            fprintf(stderr, "at line %llu column %llu, %s\n",
                    ex.getColumnNumber(), ex.getLineNumber(), msg);
            XMLString::release(&msg);
        }

    public:
        void warning(const SAXParseException& ex)
        {
            reportParseException(ex);
        }

        void error(const SAXParseException& ex)
        {
            reportParseException(ex);
        }

        void fatalError(const SAXParseException& ex)
        {
            reportParseException(ex);
        }

        void resetErrors()
        {
        }
    };

    bool ReadFormula::validateSchema(const std::string &formulaFilePath) {
        string schemaFilePath = "Formula.xsd"; // todo: we need c++ 17
        XercesDOMParser domParser;
        if (domParser.loadGrammar(schemaFilePath.c_str(), Grammar::SchemaGrammarType) == NULL) {
            throw ("Couldn't load schema");
        }


        ParserErrorHandler parserErrorHandler;

        domParser.setErrorHandler(&parserErrorHandler);
        domParser.setValidationScheme(XercesDOMParser::Val_Always);
        domParser.setDoNamespaces(true);
        domParser.setDoSchema(true);
        domParser.setValidationSchemaFullChecking(true);

        domParser.setExternalNoNamespaceSchemaLocation(schemaFilePath.c_str());

        domParser.parse(formulaFilePath.c_str());
        return (domParser.getErrorCount() == 0);
    }

    xercesc::DOMElement* ReadFormula::nextChildElement(xercesc::DOMElement *formulaNode, std::string tagName, int count) {
        DOMNodeList *children = formulaNode->getChildNodes();
        int c = 0;
        for (XMLSize_t xx = 0; xx < children->getLength(); ++xx) {
            DOMNode *currentNode = children->item(xx);
            if (currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                auto currentElement = dynamic_cast< xercesc::DOMElement * >( currentNode );
                if (XMLString::equals(currentElement->getTagName(), XMLString::transcode(tagName.c_str()))) {
                    c++;
                    if (c == count) {
                        return currentElement;
                    }
                }
            }
        }
        return NULL;
    }

    Formula ReadFormula::readFormula(const std::string &formulaFilePath) {

        // Check if file is valid
        struct stat fileStatus;
        errno = 0;
        if (stat(formulaFilePath.c_str(), &fileStatus) == -1) {
            if (errno == ENOENT)
                throw (std::runtime_error("Path file_name does not exist, or path is an empty string."));
            else if (errno == ENOTDIR)
                throw (std::runtime_error("A component of the path is not a directory."));
            else if (errno == ELOOP)
                throw (std::runtime_error("Too many symbolic links encountered while traversing the path."));
            else if (errno == EACCES)
                throw (std::runtime_error("Permission denied."));
            else if (errno == ENAMETOOLONG)
                throw (std::runtime_error("File can not be read."));
        }

        if (!validateSchema(formulaFilePath))
            throw (runtime_error("Invalid XML Schema"));

        // Configure DOM parser.
        xmlParser->setValidationScheme(XercesDOMParser::Val_Never);
        xmlParser->setDoNamespaces(false);
        xmlParser->setDoSchema(false);
        xmlParser->setLoadExternalDTD(false);

        try {
            xmlParser->parse(formulaFilePath.c_str());

            // no need to free this pointer - owned by the parent parser object
            DOMDocument *xmlDoc = xmlParser->getDocument();

            DOMElement *elementRoot = xmlDoc->getDocumentElement();
            if (!elementRoot) throw (std::runtime_error("Empty XML document."));

            auto currentElement = this->nextChildElement(elementRoot, "Formula");

            if(currentElement != NULL) {
                return Formula(parseFormula(currentElement));
            } else {
                currentElement = this->nextChildElement(elementRoot, "UntilFormula");

                auto preElement = this->nextChildElement(currentElement, "PreFormula");
                auto goalElement = this->nextChildElement(currentElement, "GoalFormula");
                auto preFormula = this->parseFormula(preElement);
                auto goalFormula = this->parseFormula(goalElement);

                double maxtime = 0;
                DOMNamedNodeMap *attributes = currentElement->getAttributes();
                for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                    DOMNode *attribute = attributes->item(i);
                    if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("maxtime"))) {
                        maxtime = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                    }
                }

                return Formula(Formula(std::make_shared<Until>(preFormula, goalFormula, maxtime)));
            }


        }
        catch (xercesc::XMLException &e) {
            // char* message = xercesc::XMLString::transcode( e.getMessage() );
            throw std::runtime_error("Error parsing XML file.");
        }
    }

    std::pair<std::string, double> ReadFormula::atomicAttributes(xercesc::DOMElement *formulaNode) {
        std::string place;
        double value;
        DOMNamedNodeMap *attributes = formulaNode->getAttributes();
        for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
            DOMNode *attribute = attributes->item(i);
            if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("place"))) {
                place = XMLString::transcode(attribute->getNodeValue());
            } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("value"))) {
                value = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
            }
        }
        return std::pair(place, value);
    }

    Formula ReadFormula::parseFormula(xercesc::DOMElement *formulaNode) {
        DOMNodeList *children = formulaNode->getChildNodes();
        for (XMLSize_t xx = 0; xx < children->getLength(); ++xx) {
            DOMNode *currentNode = children->item(xx);
            if (currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                auto currentElement = dynamic_cast< xercesc::DOMElement * >( currentNode );
                if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("DiscreteAtomicProperty"))) {
                    std::pair<string, double> attributes = this->atomicAttributes(currentElement);
                    return Formula(std::make_shared<DiscreteAtomicProperty>(attributes.first, attributes.second));
                } else if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("ContinuousAtomicProperty"))) {
                    std::pair<string, double> attributes = this->atomicAttributes(currentElement);
                    return Formula(std::make_shared<ContinuousAtomicProperty>(attributes.first, attributes.second));
                } else if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("Conjunction"))) {
                    auto firstElement = this->nextChildElement(currentElement, "Formula",1);
                    auto secondElement = this->nextChildElement(currentElement, "Formula",2);
                    auto firstFormula = this->parseFormula(firstElement);
                    auto secondFormula = this->parseFormula(secondElement);
                    return Formula(std::make_shared<Conjunction>(firstFormula, secondFormula));
                } else {
                    auto element = this->nextChildElement(currentElement, "Formula");
                    auto formula = this->parseFormula(element);
                    return Formula(std::make_shared<Negation>(formula));
                }
            }
        }
        throw std::runtime_error("Error parsing XML file.");
    }
}