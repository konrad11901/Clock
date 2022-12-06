#pragma once

#include <random>

class RandomGenerator {
public:
    RandomGenerator();

    int GetRandomNumber(int min, int max);
private:
    std::random_device random_device;
    std::default_random_engine random_engine;
};