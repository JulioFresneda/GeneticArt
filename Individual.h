#pragma once
#include "Gene.h"
#include "RandomHelper.h"
#include <memory>
#include <vector>
#include <random>
#include <algorithm>

class Individual {
public:
    std::vector<Gene> dna;
    double fitness;

    Individual() : fitness(0.0) {}
    ~Individual() = default;

    Individual(const Individual& other) {
        this->fitness = other.fitness;
        // Reserve space for efficiency
        this->dna.reserve(other.dna.size());

        for (const auto& gene : other.dna) {
            this->dna.push_back(gene.clone());
        }
    }

    Individual& operator=(const Individual& other) {
        if (this == &other) {
            return *this;
        }
        this->dna.clear();
        this->fitness = other.fitness;
        this->dna.reserve(other.dna.size());
        for (const auto& gene : other.dna) {
            this->dna.push_back(gene.clone());
        }
        
        return *this;
    }

    void add_random_gene(Random& rand, int img_width, int img_height, int max_size, ShapeType shape) {
        int x = rand.getInt(0, img_width - 1);
        int y = rand.getInt(0, img_height - 1);

        Color c = {
            (uint8_t)rand.getInt(0, 255),
            (uint8_t)rand.getInt(0, 255),
            (uint8_t)rand.getInt(0, 255),
            (uint8_t)rand.getInt(0, 100) 
        };

        int s = rand.getInt(1, max_size);
        dna.push_back(Gene(x, y, c, shape, s));
    }

    void delete_random_gene(Random& rand) {
        if (dna.empty()) return;
        int gene_index = rand.getInt(0, static_cast<int>(dna.size()) - 1);
        dna.erase(dna.begin() + gene_index);
    }

    void mutate_random_gene(Random& rand, int img_width, int img_height) {
        if (dna.empty()) return;
        int gene_index = rand.getInt(0, static_cast<int>(dna.size()) - 1);
        Gene& gene = dna[gene_index];

        // Randomly choose mutation type
        switch (rand.getInt(0, 2)) {
            case 0:
                gene.mutatePosition(rand, img_width, img_height);
                break;
            case 1:
                gene.mutateColor(rand);
                break;
            case 2:
                gene.mutateLength(rand);
                break;
        }
    }

};