#include "SingularAutomatonWriter.h"


namespace hpnmg {
    using namespace std;

    void SingularAutomatonWriter::writeAutomaton(shared_ptr<SingularAutomaton> automaton, string automatonname) {
        string filename = automatonname + ".jani";
        outputFile.open(filename);

        outputFile << "{\n"
                   << "\t\"jani-version\": 1,\n"
                   << "\t\"name\": \"" << filename << "\",\n"
                   << "\t\"type\": \"sha\",\n"
                   << "\t\"features\": [ \"derived-operators\" ],\n";


        SingularAutomaton::singleton initX = automaton->getInitialContinuous();
        SingularAutomaton::singleton initC = automaton->getInitialDeterministic();
        SingularAutomaton::singleton initG = automaton->getInitialGeneral();
        if (initX.size() + initC.size() + initG.size() > 0) {
            outputFile << "\t\"variables\": [\n";
            writeVariables(initX, initC, initG);
            outputFile << "\t],\n";
        }


        outputFile << "\t\"automata\": [\n"
                   << "\t\t{\n";

        writeAutomata(move(automaton), automatonname);

        outputFile << "\t\t}\n"
                   << "\t],\n"
                   << "\t\"system\": {\n"
                   << "\t\t\"elements\": [\n"
                   << "\t\t\t{\n"
                   << "\t\t\t\t\"automaton\": \"" << automatonname << "\"\n"
                   << "\t\t\t}\n"
                   << "\t\t]\n"
                   << "\t}\n"
                   << "\n"
                   << "}";

        outputFile.close();
    }

    void SingularAutomatonWriter::writeAutomata(shared_ptr<SingularAutomaton> automaton, string automatonname) {
        outputFile << "\t\t\t\"name\": \"" << automatonname << "\",\n";

        writeLocations(automaton);

        outputFile << "\t\t\t\"initial-locations\": [";
        bool isFirstVar = true;
        for (shared_ptr<SingularAutomaton::Location> initialLocation : automaton->getInitialLocations() ) {
            outputFile << (isFirstVar ? "" : ",");
            isFirstVar = false;
            outputFile << "\"L" << initialLocation->getLocationId() << "\"";
        }
        outputFile << "],\n";

        writeEdges(automaton);
    }

    void SingularAutomatonWriter::writeVariables(SingularAutomaton::singleton initX, SingularAutomaton::singleton initC,
                                                 SingularAutomaton::singleton initG) {
        bool isFirstVar = true;

        for (int i = 1; i <= initX.size(); i++) {
            outputFile << (isFirstVar ? "" : ",\n");
            isFirstVar = false;
            outputFile << "\t\t{\n"
                       << "\t\t\t\"name\": \"x" << i << "\",\n"
                       << "\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\"initial-value\": " << initX[i - 1] << "\n"
                       << "\t\t}";
        }
        for (int i = 1; i <= initC.size(); i++) {
            outputFile << (isFirstVar ? "" : ",\n");
            isFirstVar = false;
            outputFile << "\t\t{\n"
                       << "\t\t\t\"name\": \"c" << i << "\",\n"
                       << "\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\"initial-value\": " << initC[i - 1] << "\n"
                       << "\t\t}";
        }
        for (int i = 1; i <= initG.size(); i++) {
            outputFile << (isFirstVar ? "" : ",\n");
            isFirstVar = false;
            outputFile << "\t\t{\n"
                       << "\t\t\t\"name\": \"g" << i << "\",\n"
                       << "\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\"initial-value\": " << initG[i - 1] << "\n"
                       << "\t\t}";
        }
        outputFile << "\n";
    }

    void SingularAutomatonWriter::writeLocations(shared_ptr<SingularAutomaton> automaton) {
        outputFile << "\t\t\t\"locations\": [\n";

        bool isFirstLoc = true;
        for (const shared_ptr<SingularAutomaton::Location>& location : automaton->getLocations()) {
            outputFile << (isFirstLoc ? "" : ",\n");
            isFirstLoc = false;
            outputFile << "\t\t\t\t{\n"
                       << "\t\t\t\t\t\"name\": \"L" << location->getLocationId() << "\",\n"
                       << "\t\t\t\t\t\"time-progress\": {\n";

            SingularAutomaton::singleton actX = location->getActivitiesContinuous();
            vector<bool> actC = location->getActivitiesDeterministic();
            vector<short int> actG = location->getActivitiesGeneral();
            SingularAutomaton::rectangularSet invX = location->getInvariantsContinuous();
            SingularAutomaton::rectangularSet invC = location->getInvariantsDeterministic();
            SingularAutomaton::rectangularSet invG = location->getInvariantsGeneral();
            if (actX.size() + actC.size() + actG.size() + invX.size() + invC.size() + invG.size() > 0) {
                outputFile << "\t\t\t\t\t\t\"exp\": {\n";
                writeExpression(actX, actC, actG, invX, invC, invG);
                outputFile << "\t\t\t\t\t\t}\n";
            }

            outputFile << "\t\t\t\t\t}\n"
                       << "\t\t\t\t}";
        }

        outputFile << "\n"
                   << "\t\t\t],\n";
    }


    void SingularAutomatonWriter::writeEdges(shared_ptr<SingularAutomaton> automaton) {
        outputFile << "\t\t\t\"edges\": [\n";

        bool isFirstEdge = true;
        for (const shared_ptr<SingularAutomaton::Location>& location : automaton->getLocations()) {
            for (const shared_ptr<SingularAutomaton::Transition>& transition : location->getOutgoingTransitions()) {
                outputFile << (isFirstEdge ? "" : ",\n");
                isFirstEdge = false;
                outputFile << "\t\t\t\t{\n"
                           << "\t\t\t\t\t\"location\": \"L" << location->getLocationId() << "\",\n";
                if (transition->getType() == Continuous || transition->getType() == Timed || transition->getType() == General) {
                    writeGuard(transition);
                }
                outputFile << "\t\t\t\t\t\"destinations\": [\n"
                           << "\t\t\t\t\t\t{\n"
                           << "\t\t\t\t\t\t\t\"location\": \"L" << transition->getSuccessorLocation()->getLocationId()
                           << "\"";
                if (transition->getType() == Timed || transition->getType() == General || transition->getType() == Root) {
                    writeAssignments(transition, automaton);
                }
                outputFile << "\n"
                           << "\t\t\t\t\t\t}\n"
                           << "\t\t\t\t\t]\n"
                           << "\t\t\t\t}";
            }
        }

        outputFile << "\n"
                   << "\t\t\t]\n";
    }

    void SingularAutomatonWriter::writeGuard(shared_ptr<SingularAutomaton::Transition> transition) {

        if (transition->getType() == Continuous || transition->getType() == Timed) {
            outputFile << "\t\t\t\t\t\"guard\": {\n"
                       << "\t\t\t\t\t\t\"exp\": {\n";

            string var = (transition->getType() == Continuous ? "\"x" : "\"c") + to_string(transition->getVariableIndex() + 1) + "\"";
            double value = transition->getValueGuardCompare();
            for (const string &line : op_bin(EQUAL, var, to_string(value))) {
                outputFile << "\t\t\t\t\t\t\t" << line << "\n";
            }
            outputFile << "\t\t\t\t\t\t}\n"
                       << "\t\t\t\t\t},\n";

        } else if (transition->getType() == General && transition->getPredecessorLocation()->getActivitiesGeneral()[transition->getVariableIndex()] == -1){
            outputFile << "\t\t\t\t\t\"guard\": {\n"
                       << "\t\t\t\t\t\t\"exp\": {\n";

            string var = "\"g" + to_string(transition->getVariableIndex() + 1) + "\"";
            for (const string &line : op_bin(EQUAL, var, to_string(0.0))) {
                outputFile << "\t\t\t\t\t\t\t" << line << "\n";
            }
            outputFile << "\t\t\t\t\t\t}\n"
                       << "\t\t\t\t\t},\n";
        }
    }

    void SingularAutomatonWriter::writeAssignments(shared_ptr<SingularAutomaton::Transition> transition, shared_ptr<SingularAutomaton> automaton) {
        string var = (transition->getType() == Timed ? "c" : "g") + to_string(transition->getVariableIndex() + 1);

        vector<long> samplingVariables = transition->getSamplingVariables();
        bool comma = false;

        if (transition->getType() == Timed || transition->getType() == Root || transition->getPredecessorLocation()->getActivitiesGeneral()[transition->getVariableIndex()] == 1 || samplingVariables.size()>0) {

            outputFile << ",\n"
                       << "\t\t\t\t\t\t\t\"assignments\": [\n";

            if (transition->getType() == Timed || (transition->getType() != Root && transition->getPredecessorLocation()->getActivitiesGeneral()[transition->getVariableIndex()] == 1)) {

                outputFile << "\t\t\t\t\t\t\t\t{\n"
                           << "\t\t\t\t\t\t\t\t\t\"ref\": \"" << var << "\",\n"
                           << "\t\t\t\t\t\t\t\t\t\"value\": 0\n"
                           << "\t\t\t\t\t\t\t\t}";

                comma = true;
            }


            for (long index : samplingVariables) {

                var = "g" + to_string(index + 1);
                string dist = "";
                string args = "";
                pair<string, map<string, float>> distribution = automaton->getDistributionsNormalized()[index];

                if (distribution.first == "normal" || distribution.first == "foldednormal") {
                    dist = "Normal";
                    args = to_string((double) distribution.second.at("mu")) + string(", ") + to_string((double) distribution.second.at("sigma"));
                } else if (distribution.first == "uniform") {
                    dist = "Uniform";
                    args = to_string((double) distribution.second.at("a")) + string(", ") + to_string((double) distribution.second.at("b"));
                } else if (distribution.first == "exp") {
                    dist = "Exponential";
                    args = to_string((double) distribution.second.at("lambda"));
                }

                if (comma)
                    outputFile << ",";

                outputFile << "\n\t\t\t\t\t\t\t\t{\n"
                           << "\t\t\t\t\t\t\t\t\t\"ref\": \"" << var << "\",\n"
                           << "\t\t\t\t\t\t\t\t\t\"value\": {\n"
                           << "\t\t\t\t\t\t\t\t\t\t\"distribution\": \"" << dist << "\",\n"
                           << "\t\t\t\t\t\t\t\t\t\t\"args\": [" << args << "]\n"
                           << "\t\t\t\t\t\t\t\t\t}\n"
                           << "\t\t\t\t\t\t\t\t}";

                comma = true;
            }

            outputFile << "\n\t\t\t\t\t\t\t]";
        }
    }

/*    void SingularAutomatonWriter::writeAssignmentDistribution(shared_ptr<SingularAutomaton::Transition> transition, pair<string, map<string, float>> distribution) {
        string var = "g" + to_string(transition->getVariableIndex()+1);
        string dist = "";
        string args = "";

        if (distribution.first == "normal" || distribution.first == "foldednormal") {
            dist = "Normal";
            args = to_string((double)distribution.second.at("mu")) + string(", ") + to_string((double)distribution.second.at("sigma"));
        } else if (distribution.first == "uniform") {
            dist = "Uniform";
            args = to_string((double)distribution.second.at("a")) + string(", ") + to_string((double)distribution.second.at("b"));
        }  else if (distribution.first ==  "exp"){
            dist = "Exponential";
            args = to_string((double)distribution.second.at("lambda"));
        }

        outputFile << ",\n"
                   << "\t\t\t\t\t\t\t\"assignments\": [\n"
                   << "\t\t\t\t\t\t\t\t{\n"
                   << "\t\t\t\t\t\t\t\t\t\"ref\": \"" << var << "\",\n"
                   << "\t\t\t\t\t\t\t\t\t\"value\": {\n"
                   << "\t\t\t\t\t\t\t\t\t\"distribution\": \"" << dist << "\"\n"
                   << "\t\t\t\t\t\t\t\t\t\"args\": [" << args << "]\n"
                   << "\t\t\t\t\t\t\t\t\t}\n"
                   << "\t\t\t\t\t\t\t\t}\n"
                   << "\t\t\t\t\t\t\t]";
    }*/

    void SingularAutomatonWriter::SingularAutomatonWriter::writeExpression(SingularAutomaton::singleton actX,
                                                                           vector<bool> actC, vector<short int> actG,
                                                                           SingularAutomaton::rectangularSet invX,
                                                                           SingularAutomaton::rectangularSet invC, SingularAutomaton::rectangularSet invG) {
        for (const string& line : expressionToString(actX, actC, actG, invX, invC, invG)) {
            outputFile << "\t\t\t\t\t\t\t" << line << "\n";
        }
    }

    vector<string> SingularAutomatonWriter::SingularAutomatonWriter::expressionToString(
            SingularAutomaton::singleton actX, vector<bool> actC, vector<short int> actG,
            SingularAutomaton::rectangularSet invX, SingularAutomaton::rectangularSet invC, SingularAutomaton::rectangularSet invG) {

        vector<string> exp;
        for (int i = 0; i < actX.size(); i++) {
            if (exp.empty()) {
                exp = singletonIndexToString(actX, "x", i);
            } else {
                exp = op_bin(AND, exp, singletonIndexToString(actX, "x", i));
            }
        }
        for (int i = 0; i < actC.size(); i++) {
            if (exp.empty()) {
                exp = boolIndexToString(actC, "c", i);
            } else {
                exp = op_bin(AND, exp, boolIndexToString(actC, "c", i));
            }
        }
        for (int i = 0; i < actG.size(); i++) {
            if (exp.empty()) {
                exp = shortIntIndexToString(actG, "g", i);
            } else {
                exp = op_bin(AND, exp, shortIntIndexToString(actG, "g", i));
            }
        }
        for (int i = 0; i < invX.size(); i++) {
            if (invX[i].first != UNLIMITED) {
                if (exp.empty()) {
                    exp = rectangularSetIndexToString(invX, "x", i);
                } else {
                    exp = op_bin(AND, exp, rectangularSetIndexToString(invX, "x", i));
                }
            }
        }
        for (int i = 0; i < invC.size(); i++) {
            if (invC[i].first != UNLIMITED) {
                if (exp.empty()) {
                    exp = rectangularSetIndexToString(invC, "c", i);
                } else {
                    exp = op_bin(AND, exp, rectangularSetIndexToString(invC, "c", i));
                }
            }
        }
        for (int i = 0; i < invG.size(); i++) {
            if (invG[i].first != UNLIMITED) {
                if (exp.empty()) {
                    exp = rectangularSetIndexToString(invG, "g", i);
                } else {
                    exp = op_bin(AND, exp, rectangularSetIndexToString(invG, "g", i));
                }
            }
        }
        return exp;
    }

    vector<string>
    SingularAutomatonWriter::singletonIndexToString(SingularAutomaton::singleton singleton, string name, int index) {
        return vector<string>{op_bin(EQUAL, op_der("\"" + name + to_string(index + 1) + "\""), to_string(singleton[index]))};
    }

    vector<string> SingularAutomatonWriter::boolIndexToString(vector<bool> boolean, string name, int index) {
            return vector<string>{op_bin(EQUAL, op_der("\"" + name + to_string(index + 1) + "\""), to_string(boolean[index]))};
    }

    vector<string> SingularAutomatonWriter::shortIntIndexToString(vector<short int> integer, string name, int index) {
            return vector<string>{op_bin(EQUAL, op_der("\"" + name + to_string(index + 1) + "\""), to_string(integer[index]))};
    }

    vector<string>
    SingularAutomatonWriter::rectangularSetIndexToString(SingularAutomaton::rectangularSet rectangularSet, string name,
                                                         int index) {
        invariantOperator invOperator = rectangularSet[index].first;
        double value = rectangularSet[index].second;
        switch (invOperator) {
            case LOWER_EQUAL:
                return vector<string>{op_bin(LEQ, "\"" + name + to_string(index + 1) + "\"", to_string(value))};
            case GREATER_EQUAL:
                return vector<string>{op_bin(LEQ, to_string(value), "\"" + name + to_string(index + 1) + "\"")};
            case UNLIMITED:
                return vector<string>();
        }
    }

    vector<string> SingularAutomatonWriter::op_bin(string binop, vector<string> leftSide, vector<string> rightSide) {
        vector<string> result{OP + binop + ","};
        for (const string& line : left(leftSide)) {
            result.push_back(line);
        }
        for (const string& line : right(rightSide)) {
            result.push_back(line);
        }
        return result;
    }

    vector<string> SingularAutomatonWriter::op_bin(string binop, vector<string> leftSide, string rightSide) {
        vector<string> result{OP + binop + ","};
        for (const string& line : left(leftSide)) {
            result.push_back(line);
        }
        result.push_back(right(move(rightSide)));
        return result;
    }

    vector<string> SingularAutomatonWriter::op_bin(string binop, string leftSide, string rightSide) {
        return vector<string>{OP + binop + ",",
                              left(move(leftSide)),
                              right(move(rightSide))};
    }

    vector<string> SingularAutomatonWriter::op_der(string var) {
        return vector<string>{OP + DER + ",",
                              VAR + var};
    }

    vector<string> SingularAutomatonWriter::left(vector<string> leftSide) {
        vector<string> result{LEFT + "{"};
        for (const string& line : leftSide) {
            result.push_back("\t" + line);
        }
        result.emplace_back("},");
        return result;
    }

    string SingularAutomatonWriter::left(string leftSide) {
        return LEFT + leftSide + ",";
    }

    vector<string> SingularAutomatonWriter::right(vector<string> rightSide) {
        vector<string> result{RIGHT + "{"};
        for (const string& line : rightSide) {
            result.push_back("\t" + line);
        }
        result.emplace_back("}");
        return result;
    }

    string SingularAutomatonWriter::right(string rightSide) {
        return RIGHT + rightSide;
    }


}