#include "PLTWriter.h"


using namespace xercesc;
namespace hpnmg {
    PLTWriter::PLTWriter() {
        XMLPlatformUtils::Initialize();

        domImplementation = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("core"));
    }

    void PLTWriter::writePLT(shared_ptr<ParametricLocationTree> plt, double maxTime) {
        DOMDocument *domDocument = nullptr;
        domDocument = domImplementation->createDocument(0, XMLString::transcode("PLT"), 0);

        DOMElement *rootElement = domDocument->getDocumentElement();
        rootElement->setAttribute(XMLString::transcode("maxTime"), XMLString::transcode(to_string(maxTime).c_str()));

        ParametricLocationTree::Node rootNode = plt->getRootNode();

        rootElement->appendChild(generateParametricLocationElement(plt, rootNode, domDocument));

        serializer = domImplementation->createLSSerializer();

        DOMLSOutput *output = ((DOMImplementationLS *) domImplementation)->createLSOutput();
        DOMConfiguration *configuration = serializer->getDomConfig();

        if (configuration->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
            configuration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

        target = new LocalFileFormatTarget("result_example.xml");
        output->setByteStream(target);

        serializer->write(domDocument, output);

        delete target;
        output->release();
        serializer->release();
    }

    DOMNode *PLTWriter::generateParametricLocationElement(shared_ptr<ParametricLocationTree> plt,
                                                          ParametricLocationTree::Node node, DOMDocument *domDocument) {
        DOMElement *element = domDocument->createElement(XMLString::transcode("ParametricLocation"));
        element->setAttribute(XMLString::transcode("id"), XMLString::transcode(to_string(node.getNodeID()).c_str()));

        // set attributes from Node
        ParametricLocation location = node.getParametricLocation();
        vector<double> generalDependencies = location.getSourceEvent().getGeneralDependencies();
        string timeString = formatTimes(location.getSourceEvent().getTime(), generalDependencies);
        element->setAttribute(XMLString::transcode("time"), XMLString::transcode(timeString.c_str()));
        EventType eventType = location.getSourceEvent().getEventType();
        element->setAttribute(XMLString::transcode("eventType"), XMLString::transcode(getEventName(eventType).c_str()));
        double probability = location.getConflictProbability();
        element->setAttribute(XMLString::transcode("probability"),
                              XMLString::transcode(to_string(probability).c_str()));

        // add continuous markings
        DOMElement *contMarkingsElement = domDocument->createElement(XMLString::transcode("continuousMarkings"));
        vector<vector<double>> contMarkings = location.getContinuousMarking();
        for (int i = 0; i < contMarkings.size(); ++i) {
            DOMElement *markingElement = domDocument->createElement(XMLString::transcode("marking"));
            string value = formatTimes(contMarkings[i]);
            markingElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(value.c_str()));
            contMarkingsElement->appendChild(markingElement);
        }
        element->appendChild(contMarkingsElement);

        // add drift
        DOMElement *driftsElement = domDocument->createElement(XMLString::transcode("drifts"));
        vector<double> drifts = location.getDrift();
        for (int i = 0; i < drifts.size(); ++i) {
            DOMElement *driftElement = domDocument->createElement(XMLString::transcode("drift"));
            double value = drifts[i];
            driftElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(to_string(value).c_str()));
            driftsElement->appendChild(driftElement);
        }
        element->appendChild(driftsElement);

        // add discrete marking
        DOMElement *discMarkingsElement = domDocument->createElement(XMLString::transcode("discreteMarkings"));
        vector<int> discMarkings = location.getDiscreteMarking();
        for (int i = 0; i < discMarkings.size(); ++i) {
            DOMElement *markingElement = domDocument->createElement(XMLString::transcode("marking"));
            int value = discMarkings[i];
            markingElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(to_string(value).c_str()));
            discMarkingsElement->appendChild(markingElement);
        }
        element->appendChild(discMarkingsElement);

        // add deterministic clocks
        DOMElement *deterministicClocksElement = domDocument->createElement(XMLString::transcode("deterministicClocks"));
        vector<vector<double>> deterministicClocks = location.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            DOMElement *clockElement = domDocument->createElement(XMLString::transcode("clock"));
            string value = formatTimes(deterministicClocks[i]);
            clockElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(value.c_str()));
            deterministicClocksElement->appendChild(clockElement);
        }
        element->appendChild(deterministicClocksElement);

        // add general clocks
        DOMElement *generalClocksElement = domDocument->createElement(XMLString::transcode("generalClocks"));
        vector<vector<double>> generalClocks = location.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i) {
            DOMElement *clockElement = domDocument->createElement(XMLString::transcode("clock"));
            string value = formatTimes(generalClocks[i]);
            clockElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(value.c_str()));
            generalClocksElement->appendChild(clockElement);
        }
        element->appendChild(generalClocksElement);

        // add general transition fired
        DOMElement *generalTransitionsFiredElement = domDocument->createElement(XMLString::transcode("generalTransitionsFired"));
        vector<int> genFired = location.getGeneralTransitionsFired();
        for (int i = 0; i < genFired.size(); ++i) {
            DOMElement *firedElement = domDocument->createElement(XMLString::transcode("fired"));
            firedElement->setAttribute(XMLString::transcode("value"), XMLString::transcode(to_string(genFired[i]).c_str()));
            generalTransitionsFiredElement->appendChild(firedElement);
        }
        element->appendChild(generalTransitionsFiredElement);

        // add general boundaries
        DOMElement* generalBoundariesElement = domDocument->createElement(XMLString::transcode("boundaries"));
        vector<vector<vector<double>>> generalBoundariesLeft = location.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalBoundariesRight = location.getGeneralIntervalBoundRight();
        for (int i=0; i<generalBoundariesLeft.size(); ++i) {
            DOMElement* boundaryElement = domDocument->createElement(XMLString::transcode("boundaries"));
            for (int j=0; j<generalBoundariesLeft[i].size(); ++j) {
                DOMElement* boundaryFiringElement = domDocument->createElement(XMLString::transcode("boundary"));
                string valueLeft = formatTimes(generalBoundariesLeft[i][j]);
                string valueRight = formatTimes(generalBoundariesRight[i][j]);
                boundaryFiringElement->setAttribute(XMLString::transcode("leftBound"), XMLString::transcode(valueLeft.c_str()));
                boundaryFiringElement->setAttribute(XMLString::transcode("rightBound"), XMLString::transcode(valueRight.c_str()));
                boundaryElement->appendChild(boundaryFiringElement);
            }
            generalBoundariesElement->appendChild(boundaryElement);
        }
        element->appendChild(generalBoundariesElement);

        // add child nodes (this is the recursion)
        DOMElement* childNodesElement = domDocument->createElement(XMLString::transcode("childNodes"));
        vector<ParametricLocationTree::Node> childNodes = plt->getChildNodes(node);
        for (int i=0; i<childNodes.size(); ++i) {
            DOMNode* childElement = generateParametricLocationElement(plt, childNodes[i], domDocument);
            childNodesElement->appendChild(childElement);
        }
        element->appendChild(childNodesElement);

        return element;
    }

    string PLTWriter::getEventName(EventType eventType) {
        switch (eventType) {
            case EventType::Immediate :
                return "Immediate";
            case EventType::General :
                return "General";
            case EventType::Timed :
                return "Timed";
            case EventType::Continuous :
                return "Continuous";
            case EventType::Root :
                return "Root";
        }
    }

    string PLTWriter::formatTimes(double time, vector<double> generalDependencies) {
        string dependencyString;
        for (int i=0; i<generalDependencies.size(); ++i)
            dependencyString += " + " + to_string(generalDependencies[i]) + " S_" + to_string(i);
        return to_string(time) + dependencyString;
    }

    std::string PLTWriter::formatTimes(vector<double> generalDependencies) {
        vector<double> variables(generalDependencies.begin() + 1, generalDependencies.end());
        return formatTimes(generalDependencies[0], variables);
    }
}