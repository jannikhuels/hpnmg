#include "SingularAutomatonWriter.h"

using namespace std;

namespace hpnmg {

    void SingularAutomatonWriter::writeAutomaton(shared_ptr<SingularAutomaton> automaton, string automatonname) {
        string filename = automatonname + ".jani";
        outputFile.open(filename);

        outputFile << "{\n"
                   << "\t\"jani-version\": 1,\n"
                   << "\t\"name\": \"" << filename << "\",\n"
                   << "\t\"type\": \"ha\",\n"
                   << "\t\"automata\": [\n"
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

        SingularAutomaton::singleton initX = automaton->getInitialContinuous();
        SingularAutomaton::singleton initC = automaton->getInitialDeterministic();
        SingularAutomaton::singleton initG = automaton->getInitialGeneral();
        if (initX.size() + initC.size() + initG.size() > 0) {
            outputFile << "\t\t\t\"variables\": [\n";
            writeVariables(initX, initC, initG);
            outputFile << "\t\t\t],\n";
        }

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
            outputFile << "\t\t\t\t{\n"
                       << "\t\t\t\t\t\"name\": \"x" << i << "\",\n"
                       << "\t\t\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\t\t\"initial-value\": " << initX[i - 1] << "\n"
                       << "\t\t\t\t}";
        }
        for (int i = 1; i <= initC.size(); i++) {
            outputFile << (isFirstVar ? "" : ",\n");
            isFirstVar = false;
            outputFile << "\t\t\t\t{\n"
                       << "\t\t\t\t\t\"name\": \"c" << i << "\",\n"
                       << "\t\t\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\t\t\"initial-value\": " << initC[i - 1] << "\n"
                       << "\t\t\t\t}";
        }
        for (int i = 1; i <= initG.size(); i++) {
            outputFile << (isFirstVar ? "" : ",\n");
            isFirstVar = false;
            outputFile << "\t\t\t\t{\n"
                       << "\t\t\t\t\t\"name\": \"g" << i << "\",\n"
                       << "\t\t\t\t\t\"type\": \"continuous\",\n"
                       << "\t\t\t\t\t\"initial-value\": " << initG[i - 1] << "\n"
                       << "\t\t\t\t}";
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
            vector<bool> actG = location->getActivitiesGeneral();
            SingularAutomaton::rectangularSet invX = location->getInvariantsContinuous();
            SingularAutomaton::rectangularSet invC = location->getInvariantsDeterministic();
            if (actX.size() + actC.size() + actG.size() + invX.size() + invC.size() > 0) {
                outputFile << "\t\t\t\t\t\t\"exp\": {\n";
                writeExpression(actX, actC, actG, invX, invC);
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
                if (transition->getType() == Continuous || transition->getType() == Timed) {
                    writeGuard(transition);
                }
                outputFile << "\t\t\t\t\t\"destinations\": [\n"
                           << "\t\t\t\t\t\t{\n"
                           << "\t\t\t\t\t\t\t\"location\": \"L" << transition->getSuccessorLocation()->getLocationId()
                           << "\"";
                if (transition->getType() == Timed || transition->getType() == General) {
                    writeAssignments(transition);
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
        outputFile << "\t\t\t\t\t\"guard\": {\n"
                   << "\t\t\t\t\t\t\"exp\": {\n";

        string var = (transition->getType() == Continuous ? "\"x" : "\"c") + to_string(transition->getVariableIndex()+1) + "\"";
        double value = transition->getValueGuardCompare();
        for (const string& line : op_bin(EQUAL, var, to_string(value))) {
            outputFile << "\t\t\t\t\t\t\t" << line << "\n";
        }

        outputFile << "\t\t\t\t\t\t}\n"
                   << "\t\t\t\t\t},\n";
    }

    void SingularAutomatonWriter::writeAssignments(shared_ptr<SingularAutomaton::Transition> transition) {
        string var = (transition->getType() == Timed ? "c" : "g") + to_string(transition->getVariableIndex()+1);

        outputFile << ",\n"
                   << "\t\t\t\t\t\t\t\"assignments\": [\n"
                   << "\t\t\t\t\t\t\t\t{\n"
                   << "\t\t\t\t\t\t\t\t\t\"ref\": \"" << var << "\",\n"
                   << "\t\t\t\t\t\t\t\t\t\"value\": 0\n"
                   << "\t\t\t\t\t\t\t\t}\n"
                   << "\t\t\t\t\t\t\t]";
    }

    void SingularAutomatonWriter::SingularAutomatonWriter::writeExpression(SingularAutomaton::singleton actX,
                                                                           vector<bool> actC, vector<bool> actG,
                                                                           SingularAutomaton::rectangularSet invX,
                                                                           SingularAutomaton::rectangularSet invC) {
        for (const string& line : expressionToString(actX, actC, actG, invX, invC)) {
            outputFile << "\t\t\t\t\t\t\t" << line << "\n";
        }
    }

    vector<string> SingularAutomatonWriter::SingularAutomatonWriter::expressionToString(
            SingularAutomaton::singleton actX, vector<bool> actC, vector<bool> actG,
            SingularAutomaton::rectangularSet invX, SingularAutomaton::rectangularSet invC) {

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
                exp = boolIndexToString(actG, "g", i);
            } else {
                exp = op_bin(AND, exp, boolIndexToString(actG, "g", i));
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
        return exp;
    }

    vector<string>
    SingularAutomatonWriter::singletonIndexToString(SingularAutomaton::singleton singleton, string name, int index) {
        return vector<string>{op_bin(EQUAL, op_der("\"" + name + to_string(index + 1) + "\""), to_string(singleton[index]))};
    }

    vector<string> SingularAutomatonWriter::boolIndexToString(vector<bool> boolean, string name, int index) {
        return vector<string>{op_bin(EQUAL, op_der("\"" + name + to_string(index + 1) + "\""), to_string(boolean[index]))};
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