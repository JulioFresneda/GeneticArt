#include "Individual.h"
#include "Draw.h"

class GeneticAlgorithm
{
public:

    GeneticAlgorithm(int populationSize, int imgWidth, int imgHeight, int maxGeneSize, int generations, ShapeType shapeType)
    {
        this->populationSize = populationSize;
        this->imgWidth = imgWidth;
        this->imgHeight = imgHeight;
        this->maxGeneSize = maxGeneSize;
        this->generations = generations;
        this->shapeType = shapeType;

        initializePopulation();
    }

    void DrawBetterIndividual()
    {
        // Placeholder: In a full implementation, this would find and draw the best individual
        if (!population.empty()) {
            Draw drawer(imgWidth, imgHeight, population[0]);
        }
    }


private:
    int populationSize;
    int imgWidth;
    int imgHeight;
    int maxGeneSize;
    int generations;
    ShapeType shapeType;

    std::vector<Individual> population;
    Random rand;
    
    void initializePopulation(){
        for (int i = 0; i < populationSize; ++i) {
            Individual individual;
            individual.add_random_gene(rand, imgWidth, imgHeight, maxGeneSize, shapeType);
            population.push_back(individual);
        }
    }

};