#include <iostream>
#include "GeneticAlgorithm.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    // Write the parameter names
    GeneticAlgorithm ga(2, 800, 600, 50, 1000, ShapeType::Circle);
    ga.DrawBetterIndividual();




    return 0;
}
