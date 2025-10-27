#pragma once
#include <random>


/**
 * @brief A simple helper class to wrap C++'s modern <random> library.
 *
 * This class holds the random number generator and its seed, so you can
 * create one instance of it and pass it around by reference to any
 * function that needs to generate random numbers.
 */
class Random {
public:
    /**
     * @brief Constructor: Seeds the high-quality Mersenne Twister engine
     * using a non-deterministic hardware-based seed.
     */
    Random() : gen(rd()) {} // Seed the generator

    /**
     * @brief Get a random integer in the inclusive range [min, max].
     */
    int getInt(int min, int max) {
        // Create a distribution (this is the "dice")
        std::uniform_int_distribution<> dis(min, max);
        // "Roll the dice" using our generator
        return dis(gen);
    }

    /**
     * @brief Get a random double in the range [min, max).
     * By default, this is [0.0, 1.0).
     */
    double getDouble(double min = 0.0, double max = 1.0) {
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }

private:
    std::random_device rd;  // Hardware-based non-deterministic seed
    std::mt19937 gen;       // The high-quality random number engine
};