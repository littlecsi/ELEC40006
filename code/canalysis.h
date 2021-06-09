#define M_PI 3.141592653589793238463
#define ELEC_Vt 25e-03
#define ELEC_Is 0.3e-06
#define ELEC_Vbe 0.7

#include <iostream>
#include <vector>
#include <fstream>
#include <complex>
#include <cmath>
#include <math.h>
#include <string.h>

typedef std::vector<std::vector<double>> Mat;

using namespace std::complex_literals;

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

// -------------------- List of functions
void ReadFile(std::vector<Component>*, Option*, std::string);
void PrintCircuit(std::vector<Component>, Option);
std::string FindOutputNode(std::vector<Component>);
std::vector<Component> SmallSignalEquivalent(std::vector<Component>, int);
double CapCond(std::string, int);
double IndCond(std::string, int);
double ResCond(std::string, int);
double DioCond(std::string, int);
double ConvertUnit(std::string value);
std::vector<std::string> ConvertNode(std::vector<std::string>);
double ftow(int);
// ------------------------------------------------------------

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

std::vector<Component> SmallSignalEquivalent(std::vector<Component> circuit, int freq) {
    // Reads a Full Circuit and returns the Small-Signal Equivalent Circuit.
    std::vector<Component> SSEM;

    for (int i = 0; i < circuit.size(); i++) {
        Component comp;

        if (circuit[i].designator[0] == 'V') {
            // Voltage Sources are open circuits.
            comp.designator = "";
            comp.value = "0";

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'I') {
            // Current Sources are open circuits.
            continue;
        }
        else if (circuit[i].designator[0] == 'R') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            comp.value = std::to_string(ResCond(circuit[i].value, freq));

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'C') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            comp.value = std::to_string(CapCond(circuit[i].value, freq));

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'L') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            comp.value = std::to_string(IndCond(circuit[i].value, freq));

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'D') {
            comp.designator = circuit[i].designator;
            comp.nodes = circuit[i].nodes;
            comp.value = ELEC_Vt / (ELEC_Is * exp(ELEC_Vbe / ELEC_Vt));

            SSEM.push_back(comp);
        }
        else if (circuit[i].designator[0] == 'Q') {
            
        }
    }
    return SSEM;
}

double CapCond(std::string value, int freq) { // Find Capacitor Conductance
    return std::real(1i * ftow(freq) * ConvertUnit(value));
}

double IndCond(std::string value, int freq) { // Find Inductor Conductance
    return std::real(1.0 / (1i * ftow(freq) * ConvertUnit(value)));
}

double ResCond(std::string value, int freq) { // Find Resistor Conductance
    return 1.0 / ConvertUnit(value);
}

double DioCond(std::string value) { // Find Resistor Conductance
    double Id = ELEC_Is * exp(0.7 / ELEC_Vt); // Assuming forward BIAS, V = 0.7V
    return Id / ELEC_Vt;
}

double ConvertUnit(std::string value) {
    // If there is no multiplier, return the value as a double
    if ((int(value[value.size() - 1]) >= 48) && (int(value[value.size() - 1]) <= 57)) {
        return std::stod(value);
    }

    double multiplier = 1.0;

    if (value[value.size() - 1] == 'p') {
        multiplier = 1e-12; 
    }
    else if (value[value.size() - 1] == 'n') {
        multiplier = 1e-09; 
    }
    else if (value[value.size() - 1] == 'u') {
        multiplier = 1e-06; 
    }
    else if (value[value.size() - 1] == 'm') {
        multiplier = 1e-03; 
    }
    else if (value[value.size() - 1] == 'k') {
        multiplier = 1e+03; 
    }
    else if (value[value.size() - 1] == 'G') {
        multiplier = 1e+09; 
    }
    else if (value.substr(value.size() - 3, 3) == "Meg") {
        multiplier = 1e+06;
        return (double)(multiplier * std::stoi(value.substr(0, value.size() - 3)));
    }
    return multiplier * std::stod(value.substr(0, value.size() - 1));
}

std::vector<std::string> ConvertNode(std::vector<std::string> nodes) {
    std::vector<std::string> result;

    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i] == "0") {
            result.push_back("0");
        }
        else if (nodes[i][0] == 'N') {
            result.push_back(nodes[i].substr(1, 3));
        }
    }

    return result;
}

double ftow(int freq) { // frequency to Angular freq
    return 2 * M_PI * freq;
}