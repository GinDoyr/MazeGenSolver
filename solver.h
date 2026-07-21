#ifndef SOLVER_H
#define SOLVER_H

#include "maze.h"
#include <vector>

/* Результат работы алгоритма решения лабиринта.
   Содержит финальный путь, порядок посещения клеток и флаг успеха.
   visitedOrder нужен для анимации — виджет рисует клетки в том порядке,
   в котором их обрабатывал алгоритм. */
struct SolvingResult {
    std::vector<Point> path;          /* Путь от старта до финиша */
    std::vector<Point> visitedOrder;  /* Клетки в порядке посещения */
    bool found;                       /* true, если путь найден */
};

/* Базовый класс для всех алгоритмов решения (BFS, A*, Дейкстра).
   Конкретные реализации наследуются и переопределяют solve(). */
class Solver {
public:
    virtual ~Solver() = default;
    virtual SolvingResult solve(const Maze& maze) = 0;
};

#endif