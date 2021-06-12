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
std::vector<Component> SmallSignalEquivalent(std::vector<Component>, int);
double CapCond(std::string, int);
double IndCond(std::string, int);
double ResCond(std::string, int);
double DioCond(std::string, int);
double ConvertUnit(std::string value);
std::vector<std::string> ConvertNode(std::vector<std::string>);
Mat GenerateCondMat(Mat, std::vector<Component>, int);
Mat InitCondMat(std::vector<Component>);
double ftow(int);
Mat InitCondMat(const std::vector<Component>);
std::vector<double> InitNodeCurrMat(Mat);
double getDeterminant(Mat);
Mat getTranspose(const Mat matrix1);
Mat getCofactor(const Mat vect);
Mat getInverse(Mat vect);
std::vector<double> getNodeVolMat(Mat, std::vector<double>);
double getTransFunc(std::vector<double>, std::string, std::string);
void printMatrix(Mat);
void printMatrix(std::vector<double>);
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

double CapCond(std::string value, int freq) { // Find Capacitor Conductance
    return std::real(1i * ftow(freq) * ConvertUnit(value));
}

double IndCond(std::string value, int freq) { // Find Inductor Conductance
    return std::real(1.0 / (1i * ftow(freq) * ConvertUnit(value)));
}

double ResCond(std::string value, int freq) { // Find Resistor Conductance
    return 1.0 / ConvertUnit(value);
}

double ResCond(double value, int freq) {
    return 1.0 / value;
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

Mat GenerateCondMat(Mat condMatrix, std::vector<Component> circuit, std::vector<double>* nodeCurrMat, int freq) {
    // Calculate Conductance for all Components and add values to the Matrix
    for (int i = 0; i < circuit.size(); i++) {
        // Convert all nodes to numerical values
        circuit[i].nodes = ConvertNode(circuit[i].nodes);

        if (circuit[i].designator[0] == 'V') {
            // Voltage Sources are open circuits.
            if (circuit[i].value.substr(0, 2) == "AC") {
                continue;
            }
            else {
                if (circuit[i].nodes[0] == "0") {
                    //  If "-" terminal of the voltage source is connected to the reference node
                    int n = std::stoi(circuit[i].nodes[i]) - 1;

                    condMatrix[n][n] = 1.0;
                    
                    for (int j = 0; j < condMatrix[0].size(); j++) {
                        if (j == n) {
                            continue;
                        }
                        condMatrix[n][j] = 0;
                    }

                    (*nodeCurrMat)[1] = std::stod(circuit[i].value);
                }
                else if (circuit[i].nodes[1] == "0") {
                    //  If "+" terminal of the voltage source is connected to the reference node
                    int n = std::stoi(circuit[i].nodes[0]) - 1;

                    condMatrix[n][n] = 1.0;
                    
                    for (int j = 0; j < condMatrix[0].size(); j++) {
                        if (j == n) {
                            continue;
                        }
                        condMatrix[n][j] = 0;
                    }
                    (*nodeCurrMat)[n] = std::stod(circuit[i].value);
                }
                else {
                    // If the voltage source is connected between two non-reference nodes
                    int n0 = std::stoi(circuit[i].nodes[0]) - 1;
                    int n1 = std::stoi(circuit[i].nodes[1]) - 1;

                    condMatrix[n0][n1] = 1.0;
                    condMatrix[n0][n1] = -1.0;
                    condMatrix[n0][n1] = 0;

                    for (int j = 0; j < condMatrix[0].size(); j++) {
                        if ((j == n0) || (j == n1)) {
                            continue;
                        }
                        condMatrix[n0][j] = 0;
                    }
                    (*nodeCurrMat)[n0] = std::stod(circuit[i].value);
                }
            }
        }
        else if (circuit[i].designator[0] == 'I') {
            // Current Sources are open circuits.
            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;

            if ((*nodeCurrMat)[n0] == 0) {
                (*nodeCurrMat)[n1] += std::stod(circuit[i].value);
            }
            else if ((*nodeCurrMat)[n0] != 0) {
                (*nodeCurrMat)[n0] -= std::stod(circuit[i].value);
                (*nodeCurrMat)[n1] += std::stod(circuit[i].value);
            }
        }
        else if (circuit[i].designator[0] == 'R') {
            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;

            condMatrix[n0][n1] = -ResCond(circuit[i].value, freq);
            condMatrix[n1][n0] = -ResCond(circuit[i].value, freq);
        }
        else if (circuit[i].designator[0] == 'C') {
            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;

            condMatrix[n0][n1] = -CapCond(circuit[i].value, freq);
            condMatrix[n1][n0] = -CapCond(circuit[i].value, freq);
        }
        else if (circuit[i].designator[0] == 'L') {
            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;

            condMatrix[n0][n1] = -IndCond(circuit[i].value, freq);
            condMatrix[n1][n0] = -IndCond(circuit[i].value, freq);
        }
        else if (circuit[i].designator[0] == 'D') {
            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;

            condMatrix[n0][n1] = -DioCond(circuit[i].value);
            condMatrix[n1][n0] = -DioCond(circuit[i].value);
        }
        else if (circuit[i].designator[0] == 'Q') {
            // Assuming Ic biased at 1mA
            // BJT models are from our labs
            // VCCS version, neglecting Early Effect
            double beta = 120;
            double Ic = 1e-03;
            double Ib;
            double rbe;
            double gm;

            int n0 = std::stoi(circuit[i].nodes[0]) - 1;
            int n1 = std::stoi(circuit[i].nodes[1]) - 1;
            int n2 = std::stoi(circuit[i].nodes[2]) - 1;

            Ib = Ic / beta;
            rbe = ELEC_Vt / Ib;
            gm = Ic / ELEC_Vt;

            condMatrix[n1][n2] = 1 / rbe;
            condMatrix[n2][n1] = 1 / rbe;

            condMatrix[n0][n2] = gm;
            condMatrix[n2][n0] = gm;
        }
    }
    return condMatrix;
}

double ftow(int freq) { // frequency to Angular freq
    return 2 * M_PI * freq;
}

Mat InitCondMat(const std::vector<Component> circuit) {
    Mat condMatrix;
    int max = 0;

    for (int i = 0; i < circuit.size(); i++) {
        std::vector<std::string> node = ConvertNode(circuit[i].nodes);

        for (int j = 0; j < node.size(); j++) {
            if (std::stoi(node[j]) > max) {
                max = std::stoi(node[j]);
            }
        }
    }
    for (int i = 0; i < (max + 1); i++) {
        std::vector<double> vec;

        for (int j = 0; j < (max + 1); j++) {
            vec.push_back(0);
        }
        condMatrix.push_back(vec);
        vec.clear();
    }
    return condMatrix;
}

std::vector<double> InitNodeCurrMat(Mat condMatrix) {
    std::vector<double> nodeCurrMat;

    for (int i = 0; i < condMatrix[0].size(); i++) {
        nodeCurrMat.push_back(0);
    }
    return nodeCurrMat;
}

double getDeterminant(Mat vect) {
    int dimension = vect.size();

    // Formula for 2x2-matrix
    if(dimension == 2) {
        return vect[0][0] * vect[1][1] - vect[0][1] * vect[1][0];
    }

    // nxn-matrix where n > 2
    double result = 0;
    int sign = 1;

    for(int i = 0; i < dimension; i++) {
        // Submatrix
        std::vector<std::vector<double>> subVect(dimension - 1, std::vector<double> (dimension - 1));
        for(int m = 1; m < dimension; m++) {
            int z = 0;

            for(int n = 0; n < dimension; n++) {
                if(n != i) {
                    subVect[m-1][z] = vect[m][n];
                    z++;
                }
            }
        }

        //recursive call
        result = result + sign * vect[0][i] * getDeterminant(subVect);
        sign = -sign;
    }
    return result;
}

Mat getTranspose(const Mat matrix1) {
    // Transpose-matrix: height = width(matrix), width = height(matrix)
    std::vector<std::vector<double>> solution(matrix1[0].size(), std::vector<double> (matrix1.size()));

    //Filling solution-matrix
    for(size_t i = 0; i < matrix1.size(); i++) {
        for(size_t j = 0; j < matrix1[0].size(); j++) {
            solution[j][i] = matrix1[i][j];
        }
    }
    return solution;
}

Mat getCofactor(const Mat vect) {
    std::vector<std::vector<double>> solution(vect.size(), std::vector<double> (vect.size()));
    std::vector<std::vector<double>> subVect(vect.size() - 1, std::vector<double> (vect.size() - 1));

    for(std::size_t i = 0; i < vect.size(); i++) {
        for(std::size_t j = 0; j < vect[0].size(); j++) {

            int p = 0;
            for(size_t x = 0; x < vect.size(); x++) {
                if(x == i) {
                    continue;
                }
                int q = 0;

                for(size_t y = 0; y < vect.size(); y++) {
                    if(y == j) {
                        continue;
                    }

                    subVect[p][q] = vect[x][y];
                    q++;
                }
                p++;
            }
            solution[i][j] = pow(-1, i + j) * getDeterminant(subVect);
        }
    }
    return solution;
}

Mat getInverse(Mat vect) {
    std::cout << "checkpoint 2" << std::endl;
    if(getDeterminant(vect) == 0) {
        std::cout << "determinant is 0" << std::endl;
        throw std::runtime_error("Determinant is 0");
    }

    double d = 1.0/getDeterminant(vect);
    std::vector<std::vector<double>> solution(vect.size(), std::vector<double> (vect.size()));

    for(size_t i = 0; i < vect.size(); i++) {
        for(size_t j = 0; j < vect.size(); j++) {
            solution[i][j] = vect[i][j]; 
        }
    }

    solution = getTranspose(getCofactor(solution));

    for(size_t i = 0; i < vect.size(); i++) {
        for(size_t j = 0; j < vect.size(); j++) {
            solution[i][j] *= d;
        }
    }
    return solution;
}

std::vector<double> getNodeVolMat(Mat invMat, std::vector<double> currMat) {
    std::vector<double> nodeVolMat;
    
    for (int i = 0; i < currMat.size(); i++) {
        double sum = 0;

        for (int j = 0; j < invMat[i].size(); j++) {
            sum += invMat[i][j] * currMat[j];
        }
        nodeVolMat.push_back(sum);
    }
    return nodeVolMat;
}

double getTransFunc(std::vector<double> nodeVolMat, std::string inputNode, std::string outputNode) {
    int n0 = std::stoi(inputNode.substr(1, 3));
    int n1 = std::stoi(outputNode.substr(1, 3));

    return 20.0 * (log(nodeVolMat[n1] / nodeVolMat[n0]) / log(10));
}

void printMatrix(Mat matrix) {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printMatrix(std::vector<double> matrix) {
    for (int i = 0; i < matrix.size(); i++) {
        std::cout << matrix[i] << " ";
    }
    std::cout << std::endl;
}
