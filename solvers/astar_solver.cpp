#include "astar_solver.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <cmath>

struct Node {
    Point point;
    int g;
    int f;

    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

int AStarSolver::heuristic(const Point& a, const Point& b) const {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

SolvingResult AStarSolver::solve(const Maze& maze) {
    SolvingResult result;
    result.found = false;

    int width = maze.getWidth();
    int height = maze.getHeight();

    Point start = maze.getStart();
    Point end = maze.getEnd();

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::unordered_map<int, int> parent;
    std::unordered_map<int, int> gScore;

    auto pointToIndex = [width](const Point& p) {
        return p.y * width + p.x;
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;

    gScore[pointToIndex(start)] = 0;
    openSet.push({start, 0, heuristic(start, end)});

    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        if (visited[current.point.y][current.point.x]) {
            continue;
        }
        visited[current.point.y][current.point.x] = true;

        result.visitedOrder.push_back(current.point);

        if (current.point == end) {
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
            if (!maze.hasWall(current.point.x, current.point.y, dir)) {
                int nx = current.point.x, ny = current.point.y;

                switch (dir) {
                case Direction::North: ny--; break;
                case Direction::South: ny++; break;
                case Direction::East:  nx++; break;
                case Direction::West:  nx--; break;
                }

                if (maze.isValid(nx, ny)) {
                    int tentativeG = current.g + 1;
                    int idx = pointToIndex({nx, ny});

                    if (!visited[ny][nx] && (gScore.find(idx) == gScore.end() || tentativeG < gScore[idx])) {
                        gScore[idx] = tentativeG;
                        parent[idx] = pointToIndex(current.point);
                        int f = tentativeG + heuristic({nx, ny}, end);
                        openSet.push({{nx, ny}, tentativeG, f});
                    }
                }
            }
        }
    }

    return result;
}
