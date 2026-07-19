#ifndef PRIM_GENERATOR_H
#define PRIM_GENERATOR_H

#include "../generator.h"
#include <random>

struct Wall {
    int x, y;
    Direction dir;
};

class PrimGenerator : public Generator {
private:
    std::mt19937 rng;

public:
    PrimGenerator(double density = 1.0, unsigned int seed = 0);
    void generate(Maze& maze) override;
};

#endif
