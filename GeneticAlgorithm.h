#include "Individual.h"
#include "Draw.h"
#include <thread>
#include <mutex>
#include <atomic>

class GeneticAlgorithm
{
public:

    GeneticAlgorithm(int populationSize, int tsSize, int elitismCount, int imgWidth, int imgHeight, std::vector<Pixel32> originalPixels, int minGeneSize, int maxGeneSize, int generations, ShapeType shapeType, BlendMode blendMode = BlendMode::AlphaOver)
    {
        this->populationSize = populationSize;
        this->imgWidth = imgWidth;
        this->imgHeight = imgHeight;
        this->originalPixels = originalPixels;
        this->minGeneSize = minGeneSize;
        this->maxGeneSize = maxGeneSize;
        this->generations = generations;
        this->shapeType = shapeType;
        this->blendMode = blendMode;
        this->tournamentSize = tsSize;
        this->elitismCount = elitismCount;

        evolve();

        


    }

    void evolve()
    {
        initializePopulation();
        evaluateFitness();

        for (int gen = 0; gen < generations; ++gen) {
            
            selection();
            mutation();
            applyElitism();
            evaluateFitness();

            std::cout << "Generation " << gen + 1 << ". Best fitness: " << population[0].fitness << " with " << population[0].dna.size() << " genes." << std::endl;
            
            if(gen % 100 == 0){
                drawPixels(imgWidth, imgHeight, renderIndividualToPixels(imgWidth, imgHeight, population[0], blendMode), "./images/Generation " + std::to_string(gen + 1) + ".tga", false);
            }
        }
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
    int tournamentSize;
    int elitismCount;
    ShapeType shapeType;
    BlendMode blendMode;
    std::vector<Pixel32> originalPixels;

    std::vector<Individual> population;
    std::vector<Individual> elite;

    Random rand;
    std::mutex cout_mutex;
    
    void initializePopulation(){
        for (int i = 0; i < populationSize; ++i) {
            Individual individual;
            for (int j = 0; j < rand.getInt(minGeneSize, maxGeneSize); ++j) {
                individual.add_random_gene(rand, imgWidth, imgHeight, maxGeneSize, shapeType);
            }
            population.push_back(individual);
        }
    }

    void evaluateFitness() {
        std::atomic<int> individual_idx = 0;
        std::atomic<int> progress_count = 0;
        unsigned int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;

        auto worker = [&]() {
            while (true) {
                int idx = individual_idx.fetch_add(1);
                if (idx >= populationSize) {
                    break;
                }
                evaluateFitnessIndividual(population[idx]);
                progressBar(progress_count.fetch_add(1) + 1);
            }
        };

        for (unsigned int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        });
        std::cout << "Best fitness: " << population[0].fitness << std::endl;
    }
    
    void evaluateFitnessIndividual(Individual& individual) {
        std::vector<Pixel32> individualPixels = renderIndividualToPixels(imgWidth, imgHeight, individual, blendMode);
        double fitness = 0.0;
        for (size_t i = 0; i < originalPixels.size(); ++i) {
            const Pixel32& originalPixel = originalPixels[i];
            const Pixel32& individualPixel = individualPixels[i];
            fitness += std::abs(originalPixel.r - individualPixel.r);
            fitness += std::abs(originalPixel.g - individualPixel.g);
            fitness += std::abs(originalPixel.b - individualPixel.b);
            fitness += std::abs(originalPixel.a - individualPixel.a);
        }
        individual.fitness = fitness;

        // Penalize if close to maxGeneSize
        double percentil = static_cast<double>(individual.dna.size() - minGeneSize) / static_cast<double>(maxGeneSize - minGeneSize);
        //fitness *= (1.0 + percentil/20.0);
    }

    void progressBar(int current) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Evaluating fitness: ";
        const int barWidth = 50;
        float progress = static_cast<float>(current) / populationSize;
        std::cout << "[";
        int pos = static_cast<int>(barWidth * progress);
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %" <<  " (" << current << "/" << populationSize << ")" << "\r";
        std::cout.flush();
        if (current == populationSize) {
            std::cout << std::endl;
        }
    }

    void selection() {
        // Elitism
        std::vector<Individual> newPopulation;
        for (int i = 0; i < elitismCount; ++i) {
            elite.push_back(population[i]);
        }


        // Tournament selection
        while (newPopulation.size() < populationSize - elitismCount) {
            Individual child = crossover(tournamentSelection(tournamentSize), tournamentSelection(tournamentSize));
            newPopulation.push_back(child);
        }
        population = newPopulation;
    }

    Individual tournamentSelection(int tournamentSize) {
        Individual best;
        bool first = true;
        for (int i = 0; i < tournamentSize; ++i) {
            int index = rand.getInt(0, populationSize - 1);
            const Individual& contender = population[index];
            if (first || contender.fitness < best.fitness) {
                best = contender;
                first = false;
            }
        }
        return best;
    }



    Individual crossover(const Individual& parent1, const Individual& parent2) {
        Individual child;
        size_t size1 = parent1.dna.size();
        size_t size2 = parent2.dna.size();
        size_t minSize = std::min(size1, size2);
        size_t crossoverPoint = rand.getInt(0, static_cast<int>(minSize));

        for (size_t i = 0; i < crossoverPoint; ++i) {
            child.dna.push_back(parent1.dna[i].clone());
        }
        for (size_t i = crossoverPoint; i < size2; ++i) {
            child.dna.push_back(parent2.dna[i].clone());
        }

        return child;
    }

    void mutation() {
        int count = 0;
        for (auto& individual : population) {
            if (maybeMutate(individual)) {
                ++count;
            }
        }
        std::cout << "Mutations applied: " << count << "/" << populationSize << std::endl;
    }

    void applyElitism() {
        population.insert(population.end(), elite.begin(), elite.end());
        elite.clear();
    }


    bool maybeMutate(Individual& individual) {
        bool mutated = false;
        if (rand.getDouble(0.0, 1.0) < 0.5) { // 10% mutation rate
            individual.mutate_random_gene(rand, imgWidth, imgHeight);
            mutated = true;
        }
        if (rand.getDouble(0.0, 1.0) < 0.5) { // 10% mutation rate
            individual.add_random_gene(rand, imgWidth, imgHeight, maxGeneSize, shapeType);
            mutated = true;
        }
        if (rand.getDouble(0.0, 1.0) < 0.5) { // 50% mutation rate
            individual.delete_random_gene(rand);
            mutated = true;
        }
        return mutated;}

};