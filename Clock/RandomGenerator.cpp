#include "RandomGenerator.h"

RandomGenerator::RandomGenerator() : random_device(), random_engine(random_device()) {}

int RandomGenerator::GetRandomNumber(int min, int max) {
    std::uniform_int_distribution distribution(min, max);
    return distribution(random_engine);
}
