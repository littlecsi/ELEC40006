#include <iostream>

#include "canalysis.h"

int main() {
    // Initialise variables
    Option option;

    double transFunc;

    Mat condMatrix;
    std::vector<double> nodeCurrMat, nodeVolMat;
    std::vector<Component> circuit;

    std::string fileName;   
    std::string inputNode, outputNode;

    // Input circuit file for analysis
    std::cout << "Circuit File : ";
    std::cin >> fileName;

    ReadFile(&circuit, &option, fileName);

    std::cout << "Input Node (Nxxx) : ";
    std::cin >> inputNode;

    std::cout << "Output Node (Nxxx) : ";
    std::cin >> outputNode;

    // Circuit Analysis Begins
    std::ofstream outfile;
    outfile.open("result.csv");

    outfile << "frequency, decibels \n";

    condMatrix = InitCondMat(circuit);
    nodeCurrMat = InitNodeCurrMat(condMatrix);

    for (int freq = std::stoi(option.startFreq); freq <= ConvertUnit(option.endFreq); freq += std::stoi(option.ppd)) {
        condMatrix = GenerateCondMat(condMatrix, circuit, &nodeCurrMat, freq);

        std::vector<std::vector<double>> inverse = getInverse(condMatrix);

        nodeVolMat = getNodeVolMat(getInverse(condMatrix), nodeCurrMat);

        transFunc = getTransFunc(nodeVolMat, inputNode, outputNode);

        outfile << freq << ", " << transFunc << "\n";
    }
    
    return 0;
}