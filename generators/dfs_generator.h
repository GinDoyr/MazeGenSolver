#ifndef DFS_GENERATOR_H
#define DFS_GENERATOR_H

#include "../generator.h"
#include <random>

class DFSGenerator : public Generator {
private:
    std::mt19937 rng;

public:
    DFSGenerator(double density = 1.0, unsigned int seed = 0);
    void generate(Maze& maze) override;
};

#endif
