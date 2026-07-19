#ifndef BFS_SOLVER_H
#define BFS_SOLVER_H

#include "../solver.h"
#include <queue>

class BFSSolver : public Solver {
public:
    SolvingResult solve(const Maze& maze) override;
};

#endif
