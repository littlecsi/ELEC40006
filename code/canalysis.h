#include <iostream>
#include <vector>
#include <fstream>

struct Component {
    std::string designator;
    std::vector<std::string> nodes;
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
        if ((comp.designator[0] == 'V') || (comp.designator[0] == 'I') || (comp.designator[0] == 'R') || 
            (comp.designator[0] == 'C') || (comp.designator[0] == 'L') || (comp.designator[0] == 'D')) {
            for (int i = 0; i < 2; i++) {
                std::string val;

                infile >> val;
                comp.nodes.push_back(val);
            }
            infile >> comp.value;

            circuit->push_back(comp);
        }
        else if ((comp.designator[0] == 'Q') || (comp.designator[0] == 'D')) {
            for (int i = 0; i < 3; i++) {
                std::string val;

                infile >> val;
                comp.nodes.push_back(val);
            }
            infile >> comp.value;

            circuit->push_back(comp);
        }
        else if (comp.designator[0] == 'G') {
            for (int i = 0; i < 4; i++) {
                std::string val;

                infile >> val;
                comp.nodes.push_back(val);
            }
            infile >> comp.value;

            circuit->push_back(comp);
        }
        else if (comp.designator == ".ac") {
            infile >> option->sweepType >> option->ppd >> option->startFreq >> option->endFreq;
        }
        infile >> comp.designator;
        comp.nodes.clear();
    }

    infile.close();

    return;
}

void PrintCircuit(std::vector<Component> circuit, Option option) {
    for (int i = 0; i < circuit.size(); i++) {
        std::cout << circuit[i].designator << " ";

        for (int j = 0; j < circuit[i].nodes.size(); j++) {
            std::cout << circuit[i].nodes[j] << " ";
        }
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
            return circuit[i].nodes[0];
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

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'I') {
            continue;
        }
        else if (circuit[i].designator[0] == 'R') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            comp.value = circuit[i].value;

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'C') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            
        }
    }
    return SSEM;
}

