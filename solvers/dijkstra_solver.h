#ifndef DIJKSTRA_SOLVER_H
#define DIJKSTRA_SOLVER_H

#include "../solver.h"

class DijkstraSolver : public Solver {
public:
    SolvingResult solve(const Maze& maze) override;
};

#endif
