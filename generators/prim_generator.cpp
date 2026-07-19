#include "prim_generator.h"

PrimGenerator::PrimGenerator(double density, unsigned int seed)
    : Generator(density) {
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    }
    else {
        rng.seed(seed);
    }
}

void PrimGenerator::generate(Maze& maze) {
    int width = maze.getWidth();
    int height = maze.getHeight();

    Point start = maze.getStart();
    maze.getCell(start.x, start.y).visited = true;
    recordStep(GenerationStep::VisitCell, start.x, start.y, Direction::North);

    std::vector<Wall> walls;

    auto addWalls = [&](int x, int y) {
        if (y > 0 && !maze.getCell(x, y - 1).visited)
            walls.push_back({ x, y, Direction::North });
        if (y < height - 1 && !maze.getCell(x, y + 1).visited)
            walls.push_back({ x, y, Direction::South });
        if (x < width - 1 && !maze.getCell(x + 1, y).visited)
            walls.push_back({ x, y, Direction::East });
        if (x > 0 && !maze.getCell(x - 1, y).visited)
            walls.push_back({ x, y, Direction::West });
        };

    addWalls(start.x, start.y);

    while (!walls.empty()) {
        std::uniform_int_distribution<size_t> dist(0, walls.size() - 1);
        size_t idx = dist(rng);

        Wall wall = walls[idx];
        walls.erase(walls.begin() + idx);

        int nx = wall.x, ny = wall.y;

        switch (wall.dir) {
        case Direction::North: ny--; break;
        case Direction::South: ny++; break;
        case Direction::East:  nx++; break;
        case Direction::West:  nx--; break;
        }

        if (maze.isValid(nx, ny) && !maze.getCell(nx, ny).visited) {
            maze.removeWall(wall.x, wall.y, wall.dir);
            recordStep(GenerationStep::RemoveWall, wall.x, wall.y, wall.dir);
            maze.getCell(nx, ny).visited = true;
            recordStep(GenerationStep::VisitCell, nx, ny, Direction::North);
            addWalls(nx, ny);
        }
    }

    addExtraPassages(maze);
}
