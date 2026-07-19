#include "bfs_solver.h"
#include <algorithm>
#include <unordered_map>

SolvingResult BFSSolver::solve(const Maze& maze) {
    SolvingResult result;
    result.found = false;

    int width = maze.getWidth();
    int height = maze.getHeight();

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::unordered_map<int, int> parent;

    Point start = maze.getStart();
    Point end = maze.getEnd();

    std::queue<Point> queue;
    queue.push(start);
    visited[start.y][start.x] = true;

    auto pointToIndex = [width](const Point& p) {
        return p.y * width + p.x;
    };

    while (!queue.empty()) {
        Point current = queue.front();
        queue.pop();

        result.visitedOrder.push_back(current);

        if (current == end) {
            result.found = true;
            Point p = end;
            while (p != start) {
                result.path.push_back(p);
                int idx = pointToIndex(p);
                int parentIdx = parent[idx];
                p = {parentIdx % width, parentIdx / width};
            }
            result.path.push_back(start);
            std::reverse(result.path.begin(), result.path.end());
            return result;
        }

        std::vector<Direction> directions = {
            Direction::North, Direction::South,
            Direction::East, Direction::West
        };

        for (Direction dir : directions) {
            if (!maze.hasWall(current.x, current.y, dir)) {
                int nx = current.x, ny = current.y;

                switch (dir) {
                case Direction::North: ny--; break;
                case Direction::South: ny++; break;
                case Direction::East:  nx++; break;
                case Direction::West:  nx--; break;
                }

                if (maze.isValid(nx, ny) && !visited[ny][nx]) {
                    visited[ny][nx] = true;
                    parent[pointToIndex({nx, ny})] = pointToIndex(current);
                    queue.push({nx, ny});
                }
            }
        }
    }

    return result;
}
