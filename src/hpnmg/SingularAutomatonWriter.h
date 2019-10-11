#pragma once

#include <memory>
#include <datastructures/SingularAutomaton.h>

using namespace std;

namespace hpnmg {
    class SingularAutomatonWriter {

    private:
        const string OP = "\"op\": ";
        const string LEFT = "\"left\": ";
        const string RIGHT = "\"right\": ";
        const string AND = "\"∧\"";
        const string EQUAL = "\"=\"";
        const string LEQ = "\"≤\"";
        const string DER = "\"der\"";
        const string VAR = "\"var\": ";
        ofstream outputFile;

    public:
        void writeAutomaton(shared_ptr<SingularAutomaton> automaton, string automatonname);

    private:
        void writeAutomata(shared_ptr<SingularAutomaton> automaton, string automatonname);

        void writeVariables(SingularAutomaton::singleton initX, SingularAutomaton::singleton initC,
                SingularAutomaton::singleton initG);

        void writeLocations(shared_ptr<SingularAutomaton> automaton);

        void writeEdges(shared_ptr<SingularAutomaton> automaton);

        void writeGuard(shared_ptr<SingularAutomaton::Transition> transition);

        void writeAssignments(shared_ptr<SingularAutomaton::Transition> transition);

        void writeExpression(SingularAutomaton::singleton actX, vector<bool> actC, vector<bool> actG,
                                       SingularAutomaton::rectangularSet invX, SingularAutomaton::rectangularSet invC);

        vector<string> expressionToString(SingularAutomaton::singleton actX, vector<bool> actC, vector<bool> actG,
                SingularAutomaton::rectangularSet invX, SingularAutomaton::rectangularSet invC);

        vector<string> singletonIndexToString(SingularAutomaton::singleton singleton, string name, int index);

        vector<string> boolIndexToString(vector<bool> boolean, string name, int index);

        vector<string> rectangularSetIndexToString(SingularAutomaton::rectangularSet rectangularSet, string name, int index);

        vector<string> op_bin(string binop, vector<string> leftSide, vector<string> rightSide);

        vector<string> op_bin(string binop, vector<string> leftSide, string rightSide);

        vector<string> op_bin(string binop, string leftSide, string rightSide);

        vector<string> op_der(string var);

        vector<string> left(vector<string> leftSide);

        string left(string leftSide);

        vector<string> right(vector<string> rightSide);

        string right(string rightSide);
    };
}

