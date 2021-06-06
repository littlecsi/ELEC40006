#include <iostream>
#include <vector>
#include <fstream>

struct Component {
    std::string designator;
    std::string node0;
    std::string node1;
    std::string node2;
    std::string node3;
    std::string value;
};

struct Option {
    std::string sweepType;
    std::string ppd; // points per decade
    std::string startFreq;
    std::string endFreq;
};

void ReadFile(std::vector<Component>* circuit, Option* option, std::string filename) {

    Component comp;
    std::ifstream infile;

    infile.open(filename);

    if (!infile.is_open()) { // If file does not open, output error msg and quit.
        std::cout << "error opening file" << std::endl;
        return;
    }

    infile >> comp.designator;

    while (comp.designator != ".end") {
        // std::cout << "while loop" << std::endl;
        if ((comp.designator[0] == 'V') || (comp.designator[0] == 'I') || (comp.designator[0] == 'R') || 
            (comp.designator[0] == 'C') || (comp.designator[0] == 'L') || (comp.designator[0] == 'D')) {

            infile >> comp.node0 >> comp.node1 >> comp.value;
            comp.node2 = comp.node3 = "";
            circuit->push_back(comp);
        }
        else if ((comp.designator[0] == 'Q') || (comp.designator[0] == 'D')) {
            infile >> comp.node0 >> comp.node1 >> comp.node2 >> comp.value;
            comp.node3 = "";
            circuit->push_back(comp);
        }
        else if (comp.designator[0] == 'G') {
            infile >> comp.node0 >> comp.node1 >> comp.node2 >> comp.node3 >> comp.value;
            circuit->push_back(comp);
        }
        else if (comp.designator == ".ac") {
            infile >> option->sweepType >> option->ppd >> option->startFreq >> option->endFreq;
        }
        infile >> comp.designator;
    }

    infile.close();

    return;
}

void PrintCircuit(std::vector<Component> circuit, Option option) {
    for (int i = 0; i < circuit.size(); i++) {
        std::cout << circuit[i].designator << " ";
        std::cout << circuit[i].node0 << " ";
        std::cout << circuit[i].node1 << " ";
        std::cout << circuit[i].node2 << " ";
        std::cout << circuit[i].node3 << " ";
        std::cout << circuit[i].value << std::endl;
    }

    std::cout << option.sweepType << " ";
    std::cout << option.ppd << " ";
    std::cout << option.startFreq << " ";
    std::cout << option.endFreq << std::endl;
}

std::string FindOutputNode(std::vector<Component> circuit) {
    // Assuming there to be only one transistor in the circuit for now.
    for (int i = 0; i < circuit.size(); i++) {
        if (circuit[i].designator[0] == 'Q') {
            return circuit[i].node0;
        }
    }
    return "";
}

std::vector<Component> SmallSignalEquivalent(std::vector<Component> circuit) {
    // Reads a Full Circuit and returns the Small-Signal Equivalent Circuit.
    std::vector<Component> SSEM;

    for (int i = 0; i < circuit.size(); i++) {
        Component comp;
        if (circuit[i].designator[0] == 'V') {
            comp.designator = "";
            comp.value = "0";
        }
    }
    return SSEM;
}

