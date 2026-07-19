#ifndef ASTAR_SOLVER_H
#define ASTAR_SOLVER_H

#include "../solver.h"

class AStarSolver : public Solver {
private:
    int heuristic(const Point& a, const Point& b) const;

public:
    SolvingResult solve(const Maze& maze) override;
};

#endif
