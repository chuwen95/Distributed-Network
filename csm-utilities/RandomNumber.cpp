//
// Created by chu on 3/25/25.
//

#include "RandomNumber.h"

#include <random>

using namespace csm::utilities;

int RandomNumber::lcrc(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);

    return dis(gen);
}

int RandomNumber::loro(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min + 1, max - 1);

    return dis(gen);
}

int RandomNumber::lorc(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min + 1, max);

    return dis(gen);
}

int RandomNumber::lcro(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max - 1);

    return dis(gen);
}