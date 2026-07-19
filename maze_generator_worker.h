#ifndef MAZE_GENERATOR_WORKER_H
#define MAZE_GENERATOR_WORKER_H

#include "maze.h"
#include "maze_widget.h"
#include <vector>
#include <memory>

class MazeGeneratorWorker
{
public:
    static void generate(Maze &maze, const QString &algorithmName,
                         double density, std::vector<GenerationStep> &outSteps);
};

#endif
