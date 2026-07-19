#include "dfs_generator.h"
#include <stack>

DFSGenerator::DFSGenerator(double density, unsigned int seed)
    : Generator(density) {
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    }
    else {
        rng.seed(seed);
    }
}

void DFSGenerator::generate(Maze& maze) {
    int width = maze.getWidth();
    int height = maze.getHeight();

    std::uniform_int_distribution<int> distX(0, width - 1);
    std::uniform_int_distribution<int> distY(0, height - 1);

    Point start = {distX(rng), distY(rng)};
    maze.getCell(start.x, start.y).visited = true;
    recordStep(GenerationStep::VisitCell, start.x, start.y, Direction::North);

    std::stack<Point> stack;
    stack.push(start);

    std::vector<Direction> directions = {
        Direction::North, Direction::South,
        Direction::East, Direction::West
    };

    while (!stack.empty()) {
        Point current = stack.top();

        std::vector<Direction> available;

        for (Direction dir : directions) {
            int nx = current.x, ny = current.y;

            switch (dir) {
            case Direction::North: ny--; break;
            case Direction::South: ny++; break;
            case Direction::East:  nx++; break;
            case Direction::West:  nx--; break;
            }

            if (maze.isValid(nx, ny) && !maze.getCell(nx, ny).visited) {
                available.push_back(dir);
            }
        }

        if (!available.empty()) {
            std::uniform_int_distribution<size_t> dirDist(0, available.size() - 1);
            Direction chosenDir = available[dirDist(rng)];

            int nx = current.x, ny = current.y;
            switch (chosenDir) {
            case Direction::North: ny--; break;
            case Direction::South: ny++; break;
            case Direction::East:  nx++; break;
            case Direction::West:  nx--; break;
            }

            maze.removeWall(current.x, current.y, chosenDir);
            recordStep(GenerationStep::RemoveWall, current.x, current.y, chosenDir);
            maze.getCell(nx, ny).visited = true;
            recordStep(GenerationStep::VisitCell, nx, ny, Direction::North);
            stack.push({nx, ny});
        } else {
            stack.pop();
        }
    }

    addExtraPassages(maze);
}
