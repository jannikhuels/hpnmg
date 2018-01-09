#include "ReadHybridPetrinet.h"


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

            cerr << "XML ttolkit teardown error: " << message << endl;
            XMLString::release(&message);
        }
    }


    HybridPetriNet *ReadHybridPetrinet::readHybridPetrinet(const std::string &filepath) {
        hybridPetriNet = HybridPetriNet();

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

            for (XMLSize_t xx = 0; xx < children->getLength(); ++xx) {
                DOMNode *currentNode = children->item(xx);
                if (currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                    auto currentElement = dynamic_cast< xercesc::DOMElement * >( currentNode );
                    if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("places"))) {
                        parsePlaces(currentElement);
                    } else if (XMLString::equals(currentElement->getTagName(), XMLString::transcode("transitions"))) {
                        parseTransitions(currentElement);
                    }
                }
            }
        }
        catch (xercesc::XMLException &e) {
            // char* message = xercesc::XMLString::transcode( e.getMessage() );
            std::runtime_error("Error parsing XML file.");
        }

        return &hybridPetriNet;
    }

    void ReadHybridPetrinet::parsePlaces(DOMElement *placesNode) {
        DOMNodeList *placeNodes = placesNode->getChildNodes();

        for (XMLSize_t j = 0; j < placeNodes->getLength(); ++j) {
            DOMNode *placeNode = placeNodes->item(j);
            if (placeNode->getNodeType() && placeNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                // place is discrete place
                if (XMLString::equals(placeNode->getNodeName(), XMLString::transcode("discretePlace"))) {
                    DOMNamedNodeMap *attributes = placeNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *marking;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("marking"))) {
                            marking = attribute->getNodeValue();
                        }
                    }
                    DiscretePlace place = DiscretePlace();
                    hybridPetriNet.addDiscretePlace(place);

                    // place is continous place
                } else if (XMLString::equals(placeNode->getNodeName(), XMLString::transcode("continuousPlace"))) {
                    DOMNamedNodeMap *attributes = placeNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *capacity;
                        const XMLCh *infiniteCapacity;
                        const XMLCh *level;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("capacity"))) {
                            capacity = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(),
                                                     XMLString::transcode("infiniteCapacity"))) {
                            infiniteCapacity = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("level"))) {
                            level = attribute->getNodeValue();
                        }
                    }
                    FluidPlace place = FluidPlace();
                    hybridPetriNet.addFluidPlace(place);
                } else {
                    throw (std::runtime_error("Unknown place type."));
                }
            }
        }
    }

    void ReadHybridPetrinet::parseTransitions(DOMElement *transistionsNode) {
        DOMNodeList *transitionNodes = transistionsNode->getChildNodes();

        for (XMLSize_t j = 0; j < transitionNodes->getLength(); ++j) {
            DOMNode *transitionNode = transitionNodes->item(j);

            if (transitionNode->getNodeType() && transitionNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                // transition is deterministic transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("deterministicTransition"))) {
                    DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *discTime;
                        const XMLCh *priority;
                        const XMLCh *weight;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("discTime"))) {
                            discTime = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = attribute->getNodeValue();
                        }
                    }
                    DeterministicTransition transition = DeterministicTransition();
                    hybridPetriNet.addDeterministicTransition(transition);
                }

                // transition is fluid transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("continuousTransition"))) {
                    DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *rate;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("rate"))) {
                            rate = attribute->getNodeValue();
                        }
                    }
                    FluidTransition transition = FluidTransition();
                    hybridPetriNet.addFluidTransition(transition);
                }

                // transition is general transition todo: attribute parameter
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("generalTransition"))) {
                    DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *cdf;
                        const XMLCh *priority;
                        const XMLCh *weight;
                        const XMLCh *policy;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("cdf"))) {
                            cdf = attribute->getNodeValue();
                        }  else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("policy"))) {
                            policy = attribute->getNodeValue();
                        }
                    }
                    GeneralTransition transition = GeneralTransition();
                    hybridPetriNet.addGeneralTransition(transition);
                }

                // transition is immediate transition
                if (XMLString::equals(transitionNode->getNodeName(), XMLString::transcode("immediateTransition"))) {
                    DOMNamedNodeMap *attributes = transitionNode->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); ++i) {
                        DOMNode *attribute = attributes->item(i);
                        const XMLCh *id;
                        const XMLCh *priority;
                        const XMLCh *weight;
                        if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("id"))) {
                            id = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("priority"))) {
                            priority = attribute->getNodeValue();
                        } else if (XMLString::equals(attribute->getNodeName(), XMLString::transcode("weight"))) {
                            weight = attribute->getNodeValue();
                        }
                    }
                    ImmediateTransition transition = ImmediateTransition();
                    hybridPetriNet.addImmediateTransition(transition);
                }
            }
        }
    }

}