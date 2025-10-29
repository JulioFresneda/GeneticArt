#include <iostream>
// Ensure stb_image implementation is compiled in this translation unit
#define STB_IMAGE_IMPLEMENTATION
#include "LoadImage.h"
#include "GeneticAlgorithm.h"
#include "Draw.h"


int main() {
    std::cout << "Hello, Genetic Art!" << std::endl;

    // Load an image
    Image img = loadImage("./pic.jpg");

    int width = img.width;
    int height = img.height;

    // Render original
    auto originalPixels = imageToPixels(img);
    drawPixels(width, height, originalPixels, "", true);

    GeneticAlgorithm ga(100, 3, 1, width, height, originalPixels, 100, 5000, 10000, ShapeType::Circle, BlendMode::AlphaOver);
    Individual bestIndividual = ga.BestIndividual();

    // Render and show the best individual using free functions
    auto pixels = renderIndividualToPixels(width, height, bestIndividual, BlendMode::AlphaOver);
    drawPixels(width, height, pixels, "", true);

    

    return 0;
}
