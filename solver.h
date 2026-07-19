#ifndef SOLVER_H
#define SOLVER_H

#include "maze.h"
#include <vector>

struct SolvingResult {
    std::vector<Point> path;
    std::vector<Point> visitedOrder;
    bool found;
};

class Solver {
public:
    virtual ~Solver() = default;
    virtual SolvingResult solve(const Maze& maze) = 0;
};

#endif
