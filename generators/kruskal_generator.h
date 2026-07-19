#ifndef KRUSKAL_GENERATOR_H
#define KRUSKAL_GENERATOR_H

#include "../generator.h"
#include <vector>
#include <random>

struct Edge {
    int x1, y1, x2, y2;
    Direction dir;
};

class KruskalGenerator : public Generator {
private:
    std::mt19937 rng;
    std::vector<int> parent;
    std::vector<int> rank;

    int find(int i);
    void unite(int i, int j);

public:
    KruskalGenerator(double density = 1.0, unsigned int seed = 0);
    void generate(Maze& maze) override;
};

#endif
