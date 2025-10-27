#include "Individual.h"
#include "Draw.h"

class GeneticAlgorithm
{
public:

    GeneticAlgorithm(int populationSize, int imgWidth, int imgHeight, int minGeneSize, int maxGeneSize, int generations, ShapeType shapeType, BlendMode blendMode = BlendMode::AlphaOver)
    {
        this->populationSize = populationSize;
        this->imgWidth = imgWidth;
        this->imgHeight = imgHeight;
        this->minGeneSize = minGeneSize;
        this->maxGeneSize = maxGeneSize;
        this->generations = generations;
        this->shapeType = shapeType;
        this->blendMode = blendMode;


        initializePopulation();
    }

    Individual BestIndividual()
    {
        return population[0];
    }


private:
    int populationSize;
    int imgWidth;
    int imgHeight;
    int minGeneSize;
    int maxGeneSize;
    int generations;
    ShapeType shapeType;
    BlendMode blendMode;

    std::vector<Individual> population;
    Random rand;
    
    void initializePopulation(){
        for (int i = 0; i < populationSize; ++i) {
            Individual individual;
            for (int j = 0; j < rand.getInt(minGeneSize, maxGeneSize); ++j) {
                individual.add_random_gene(rand, imgWidth, imgHeight, maxGeneSize, shapeType);
            }
            population.push_back(individual);
        }
    }

    void evaluateFitness(Individual& individual) {
        // Placeholder: In a full implementation, this would compute the fitness of the individual
        individual.fitness = rand.getDouble(0.0, 1.0); // Random fitness for demonstration
    }

};