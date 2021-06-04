#include <iostream>
#include <vector>
#include <fstream>
#include <tuple>

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
        if ((comp.designator[0] == 'V') || (comp.designator[0] == 'I') || (comp.designator[0] == 'R') || 
            (comp.designator[0] == 'C') || (comp.designator[0] == 'L') || (comp.designator[0] == 'D')) {

            infile >> comp.node0 >> comp.node1 >> comp.value;
            comp.node2 = "";
            comp.node3 = "";
            circuit->push_back(comp);
            infile >> comp.designator;
        }
        else if ((comp.designator[0] == 'Q') || (comp.designator[0] == 'D')) {
            infile >> comp.node0 >> comp.node1 >> comp.node2 >> comp.value;
            comp.node3 = "";
            circuit->push_back(comp);
            infile >> comp.designator;
        }
        else if (comp.designator[0] == 'G') {
            infile >> comp.node0 >> comp.node1 >> comp.node2 >> comp.node3 >> comp.value;
            circuit->push_back(comp);
            infile >> comp.designator;
        }
        else if (comp.designator == ".ac") {
            infile >> option->sweepType >> option->ppd >> option->startFreq >> option->endFreq;
        }
    }

    infile.close();

    return;
}