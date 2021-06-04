#include <iostream>

#include "store.h"

int main() {
    std::vector<Component> circuit;
    Option option;
    std::string fileName;    

    std::cout << "Circuit File : ";
    std::cin >> fileName;

    ReadFile(&circuit, &option, fileName);

    for (int i = 0; i < circuit.size(); i++) {
        std::cout << circuit[i].designator << " ";
        std::cout << circuit[i].node0 << " ";
        std::cout << circuit[i].node1 << " ";
        std::cout << circuit[i].node2 << " ";
        std::cout << circuit[i].node3 << " ";
        std::cout << circuit[i].value << std::endl;
    }

    return 0;
}