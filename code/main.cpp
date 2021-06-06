#include <iostream>

#include "canalysis.h"

int main() {
    // Initialise variables
    std::vector<Component> circuit;
    Option option;
    std::string fileName;   
    std::string outputNode; 

    // Input circuit file for analysis
    std::cout << "Circuit File : ";
    std::cin >> fileName;

    ReadFile(&circuit, &option, fileName);

    // PrintCircuit(circuit, option);

    // Circuit Analysis Begins
    outputNode = FindOutputNode(circuit);

    std::cout << outputNode << std::endl;

    return 0;
}