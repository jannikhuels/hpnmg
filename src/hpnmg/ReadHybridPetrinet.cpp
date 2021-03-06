#include "ReadHybridPetrinet.h"
#include "util/statistics/Statistics.h"

using namespace xercesc;
using namespace std;
namespace hpnmg {

    ReadHybridPetrinet::ReadHybridPetrinet() {
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

/**
 *  Class destructor frees memory used to hold the XML tag and attribute definitions.
 *  It als terminates use of the xerces-C framework.
 */

    ReadHybridPetrinet::~ReadHybridPetrinet() {
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

    bool ReadHybridPetrinet::validateSchema(const string &filepath) {
        string schemaFilePath = "HPnG.xsd"; // todo: we need c++ 17
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

        domParser.parse(filepath.c_str());
        return (domParser.getErrorCount() == 0);
    }

    shared_ptr<HybridPetrinet> ReadHybridPetrinet::readHybridPetrinet(const std::string &filepath) {
        START_BENCHMARK_OPERATION("READ_HPN")
        COUNT_STATS("READ_HPN")
        hybridPetrinet = make_shared<HybridPetrinet>();

        // Check if file is valid
        struct stat fileStatus;
        errno = 0;
        if (stat(filepath.c_str(), &fileStatus) == -1) {
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

        if (!validateSchema(filepath))
            throw (runtime_error("Invalid XML Schema"));

        // Configure DOM parser.
        xmlParser->setValidationScheme(XercesDOMParser::Val_Never);
        xmlParser->setDoNamespaces(false);
        xmlParser->setDoSchema(false);
        xmlParser->setLoadExternalDTD(false);

        try {
            xmlParser->parse(filepath.c_str());

            // no need to free this pointer - owned by the parent parser object
            DOMDocument *xmlDoc = xmlParser->getDocument();

            DOMElement *elementRoot = xmlDoc->getDocumentElement();
            if (!elementRoot) throw (std::runtime_error("Empty XML document."));

            DOMNodeList *children = elementRoot->getChildNodes();

            // First parse places and transitions
            for (XMLSize_t xx = 0; xx < children->getLength(); ++xx) {
                DOMNode *currentNode = children->item(xx);
                if (currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                    auto currentElement = dynamic_cast< xercesc::DOMElement * >( currentNode );
                    if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("places"))) {
                        parsePlaces(currentElement);
                    } else if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("transitions"))) {
                        parseTransitions(currentElement);
                        parseDynamicTransitions(currentElement);
                    }
                }
            }

            // Then parse arcs (we need places and transitions)
            for (XMLSize_t xx = 0; xx < children->getLength(); ++xx) {
                DOMNode *currentNode = children->item(xx);
                if (currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                    auto currentElement = dynamic_cast< xercesc::DOMElement * >( currentNode );
                    if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("arcs"))) {
                        parseArcs(currentElement);
                    }
                }
            }

        }
        catch (xercesc::XMLException &e) {
            // char* message = xercesc::XMLString::transcode( e.getMessage() );
            std::runtime_error("Error parsing XML file.");
        }
        STOP_BENCHMARK_OPERATION("READ_HPN")
        return hybridPetrinet;
    }

    void ReadHybridPetrinet::parsePlaces(DOMElement *placesNode) {
        DOMNodeList *placeNodes = placesNode->getChildNodes();

        for (XMLSize_t j = 0; j < placeNodes->getLength(); ++j) {
            DOMNode *placeNode = placeNodes->item(j);
            if (placeNode->getNodeType() && placeNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMNamedNodeMap *attributes = placeNode->getAttributes();
                string id;
                // place is discrete place
                if (XMLString::equals(placeNode->getNodeName(), XMLString::transcode("discretePlace"))) {
                    unsigned long marking;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("marking"))) {
                            marking = strtoul(XMLString::transcode(attribute->getNodeValue()), nullptr, 0);
                        }
                    }
                    shared_ptr<DiscretePlace> place = make_shared<DiscretePlace>(id, marking);
                    hybridPetrinet->addPlace(place);

                    // place is continuous place
                } else if (XMLString::equals(placeNode->getNodeName(), XMLString::transcode("continuousPlace"))) {
                    double capacity;
                    bool infiniteCapacity;
                    double level;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("capacity"))) {
                            capacity = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(),
                                                     XMLString::transcode("infiniteCapacity"))) {
                            infiniteCapacity = !XMLString::equals(attribute->getNodeValue(), XMLString::transcode("0"));
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("level"))) {
                            level = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }
                    auto place = make_shared<ContinuousPlace>(id, capacity, level, infiniteCapacity);
                    hybridPetrinet->addPlace(place);
                } else throw (std::runtime_error("Unknown place type."));
            }
        }
    }

    void ReadHybridPetrinet::parseTransitions(DOMElement *transistionsNode) {
        DOMNodeList *transitionNodes = transistionsNode->getChildNodes();

        for (XMLSize_t j = 0; j < transitionNodes->getLength(); ++j) {
            DOMNode *transitionNode = transitionNodes->item(j);

            if (transitionNode->getNodeType() && transitionNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                string id;
                // transition is deterministic transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("deterministicTransition"))) {
                    unsigned long priority;
                    double weight;
                    double discTime;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = strtoul(XMLString::transcode(attribute->getNodeValue()), nullptr, 0);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("discTime"))) {
                            discTime = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }
                    auto transition = make_shared<DeterministicTransition>(id, priority, weight, discTime);
                    hybridPetrinet->addTransition(transition);
                }

                // transition is continuous transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("continuousTransition"))) {
                    double rate;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("rate"))) {
                            rate = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }
                    auto transition = make_shared<ContinuousTransition>(id, rate);
                    hybridPetrinet->addTransition(transition);
                }

                // transition is general transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("generalTransition"))) {
                    unsigned long priority;
                    float weight;
                    string cdf;
                    map<string, float> parameter;
                    string policy;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = strtoul(XMLString::transcode(attribute->getNodeValue()), nullptr, 0);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtof(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("cdf"))) {
                            cdf = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("cdf"))) {
                            policy = XMLString::transcode(attribute->getNodeValue());
                        }
                    }
                    DOMNodeList* parameterNodes = transitionNode->getChildNodes();
                    for (XMLSize_t k = 0; k < parameterNodes->getLength(); ++k) {
                        DOMNode* parameterNode = parameterNodes->item(k);
                        if (XMLString::equals(parameterNode->getNodeName(), XMLString::transcode("parameter"))) {
                            DOMNamedNodeMap* parameterAttributes = parameterNode->getAttributes();
                            string parameterName;
                            float value;
                            for(XMLSize_t l=0; l<parameterAttributes->getLength(); ++l) {
                                DOMNode* parameterAttribute = parameterAttributes->item(l);
                                if (XMLString::equals(parameterAttribute->getNodeName(), XMLString::transcode("name"))) {
                                    parameterName = XMLString::transcode(parameterAttribute->getNodeValue());
                                } else if (XMLString::equals(parameterAttribute->getNodeName(), XMLString::transcode("value"))) {
                                    value = strtof(XMLString::transcode(parameterAttribute->getNodeValue()), nullptr);
                                }
                            }
                            parameter[parameterName] = value;
                        }
                    }

                    auto transition = make_shared<GeneralTransition>(id, priority, weight, cdf, parameter, policy);
                    hybridPetrinet->addTransition(transition);
                }

                // transition is immediate transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("immediateTransition"))) {
                    unsigned long priority;
                    float weight;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = strtoul(XMLString::transcode(attribute->getNodeValue()), nullptr, 0);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtof(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }
                    auto transition = make_shared<ImmediateTransition>(id, priority, weight);
                    hybridPetrinet->addTransition(transition);
                }
            }
        }
    }

    void ReadHybridPetrinet::parseDynamicTransitions(DOMElement *transistionsNode) {
        DOMNodeList *transitionNodes = transistionsNode->getChildNodes();

        for (XMLSize_t j = 0; j < transitionNodes->getLength(); ++j) {
            DOMNode *transitionNode = transitionNodes->item(j);

            if (transitionNode->getNodeType() && transitionNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                string id;

                // transition is dynamic transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("dynamicTransition"))) {
                    double factor = 1;
                    double parameter;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("factor"))) {
                            factor = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("parameter"))) {
                            parameter = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }

                    double constant = 0;
                    vector<shared_ptr<ContinuousTransition>> transitions;
                    vector<double> transitionFactors;
                    DOMNodeList* dependencyNodes = transitionNode->getChildNodes();
                    for (XMLSize_t k = 0; k < dependencyNodes->getLength(); ++k) {
                        DOMNode* dependencyNode = dependencyNodes->item(k);
                        if (XMLString::equals(dependencyNode->getNodeName(), XMLString::transcode("constant"))) {
                            DOMNamedNodeMap* dependencyAttributes = dependencyNode->getAttributes();
                            double factor = 1;
                            double value;
                            for(XMLSize_t l=0; l<dependencyAttributes->getLength(); ++l) {
                                DOMNode* dependencyAttribute = dependencyAttributes->item(l);
                                if (XMLString::equals(dependencyAttribute->getNodeName(), XMLString::transcode("factor"))) {
                                    factor = strtof(XMLString::transcode(dependencyAttribute->getNodeValue()), nullptr);
                                } else if (XMLString::equals(dependencyAttribute->getNodeName(), XMLString::transcode("value"))) {
                                    value = strtof(XMLString::transcode(dependencyAttribute->getNodeValue()), nullptr);
                                }
                            }
                            constant = factor * value;
                        } else if (XMLString::equals(dependencyNode->getNodeName(), XMLString::transcode("continuousTransition"))) {
                            DOMNamedNodeMap* dependencyAttributes = dependencyNode->getAttributes();
                            for(XMLSize_t l=0; l<dependencyAttributes->getLength(); ++l) {
                                DOMNode* dependencyAttribute = dependencyAttributes->item(l);
                                if (XMLString::equals(dependencyAttribute->getNodeName(), XMLString::transcode("referenceId"))) {
                                    string transitionId = XMLString::transcode(dependencyAttribute->getNodeValue());
                                    auto continuousTransitions = hybridPetrinet->getContinuousTransitions();
                                    for (auto continuousTransition : continuousTransitions) {
                                        if (transitionId == continuousTransition.first) {
                                            transitions.push_back(continuousTransition.second);
                                        }
                                    }
                                    if (dependencyAttributes->getLength()<2) { transitionFactors.push_back(1); }
                                } else if (XMLString::equals(dependencyAttribute->getNodeName(), XMLString::transcode("factor"))) {
                                    transitionFactors.push_back(strtof(XMLString::transcode(dependencyAttribute->getNodeValue()), nullptr));
                                }
                            }

                        }
                    }

                    auto transition = make_shared<DynamicTransition>(id, factor, constant, transitions, transitionFactors, parameter);
                    hybridPetrinet->addTransition(transition);
                }
            }
        }
    }

    void ReadHybridPetrinet::parseArcs(DOMElement *arcsNode) {
        DOMNodeList *arcNodes = arcsNode->getChildNodes();

        for (XMLSize_t j = 0; j < arcNodes->getLength(); ++j) {
            DOMNode *arcNode = arcNodes->item(j);

            if (arcNode->getNodeType() && arcNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMNamedNodeMap *attributes = arcNode->getAttributes();
                string id;
                float weight;
                bool isInputArc;
                shared_ptr<Place> place;
                string transitionId;
                // arc is discrete arc
                if (XMLString::equals(arcNode->getNodeName(), XMLString::transcode("discreteArc"))) {
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtof(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("fromNode")) or
                                   XMLString::equals(attribute->getNodeName(), XMLString::transcode("toNode"))) {
                            string nodeId = XMLString::transcode(attribute->getNodeValue());
                            string nodeType = hybridPetrinet->getNodeTypeByID(nodeId);
                            if (nodeType == "place") {
                                place = hybridPetrinet->getPlaceById(nodeId);
                                isInputArc = XMLString::equals(attribute->getNodeName(),
                                                               XMLString::transcode("fromNode"));
                            } else if (nodeType == "transition") {
                                transitionId = nodeId;
                            } else throw (std::runtime_error("Discrete arc for unknown node."));
                        }
                    }
                    shared_ptr<DiscreteArc> arc = make_shared<DiscreteArc>(id, weight, place);
                    if (isInputArc) {
                        hybridPetrinet->addInputArc(transitionId, arc);
                    } else {
                        hybridPetrinet->addOutputArc(transitionId, arc);
                    }
                }

                // arc is  continuous arc
                else if (XMLString::equals(arcNode->getNodeName(), XMLString::transcode("continuousArc"))) {
                    unsigned long priority;
                    double share;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtof(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("fromNode")) or
                                   XMLString::equals(attribute->getNodeName(), XMLString::transcode("toNode"))) {
                            string nodeId = XMLString::transcode(attribute->getNodeValue());
                            string nodeType = hybridPetrinet->getNodeTypeByID(nodeId);
                            if (nodeType == "place") {
                                place = hybridPetrinet->getPlaceById(nodeId);
                                isInputArc = XMLString::equals(attribute->getNodeName(),
                                                               XMLString::transcode("fromNode"));
                            } else if (nodeType == "transition") {
                                transitionId = nodeId;
                            } else throw (std::runtime_error("Cont. arc for unknown node."));
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = strtoul(XMLString::transcode(attribute->getNodeValue()), nullptr, 0);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("share"))) {
                            share = strtod(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        }
                    }
                    auto arc = make_shared<ContinuousArc>(id, weight, place, priority, share);
                    if (isInputArc) {
                        hybridPetrinet->addInputArc(transitionId, arc);
                    } else {
                        hybridPetrinet->addOutputArc(transitionId, arc);
                    }
                }

                // arc is  guard arc
                else if (XMLString::equals(arcNode->getNodeName(), XMLString::transcode("guardArc"))) {
                    bool isInhibitor;
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = XMLString::transcode(attribute->getNodeValue());
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = strtof(XMLString::transcode(attribute->getNodeValue()), nullptr);
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("fromNode")) or
                                   XMLString::equals(attribute->getNodeName(), XMLString::transcode("toNode"))) {
                            string nodeId = XMLString::transcode(attribute->getNodeValue());
                            string nodeType = hybridPetrinet->getNodeTypeByID(nodeId);
                            if (nodeType == "place") {
                                place = hybridPetrinet->getPlaceById(nodeId);
                                isInputArc = XMLString::equals(attribute->getNodeName(),
                                                               XMLString::transcode("fromNode"));
                            } else if (nodeType == "transition") {
                                transitionId = nodeId;
                            } else throw (std::runtime_error("Guard arc for unknown node."));
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("isInhibitor"))) {
                            isInhibitor = !XMLString::equals(attribute->getNodeValue(), XMLString::transcode("0"));
                        }
                    }
                    auto arc = make_shared<GuardArc>(id, weight, place, isInhibitor);
                    if (isInputArc) {
                        hybridPetrinet->addInputArc(transitionId, arc);
                    } else { // TODO this never happens. Guard arcs are always input arcs
                        hybridPetrinet->addOutputArc(transitionId, arc);
                    }
                }
            }
        }
    }

}