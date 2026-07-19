#include "generator.h"
#include <random>
#include <vector>
#include <algorithm>

void Generator::recordStep(GenerationStep::Type type, int x, int y, Direction dir) {
    if (stepsRecorder) {
        stepsRecorder->push_back({type, x, y, dir});
    }
}

void Generator::addExtraPassages(Maze& maze) {
    if (wallDensity >= 1.0) {
        return;
    }

    int width = maze.getWidth();
    int height = maze.getHeight();

    std::vector<std::pair<int, int>> possibleWalls;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x + 1 < width && maze.hasWall(x, y, Direction::East)) {
                possibleWalls.push_back({x, y});
            }
            if (y + 1 < height && maze.hasWall(x, y, Direction::South)) {
                possibleWalls.push_back({x, y});
            }
        }
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(possibleWalls.begin(), possibleWalls.end(), rng);

    size_t wallsToRemove = static_cast<size_t>(possibleWalls.size() * (1.0 - wallDensity));

    for (size_t i = 0; i < wallsToRemove && i < possibleWalls.size(); ++i) {
        int x = possibleWalls[i].first;
        int y = possibleWalls[i].second;

        std::uniform_int_distribution<int> dirDist(0, 1);
        if (dirDist(rng) == 0 && x + 1 < width && maze.hasWall(x, y, Direction::East)) {
            maze.removeWall(x, y, Direction::East);
            recordStep(GenerationStep::RemoveWall, x, y, Direction::East);
        } else if (y + 1 < height && maze.hasWall(x, y, Direction::South)) {
            maze.removeWall(x, y, Direction::South);
            recordStep(GenerationStep::RemoveWall, x, y, Direction::South);
        }
    }
}
