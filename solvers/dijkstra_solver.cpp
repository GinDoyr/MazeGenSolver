#include "dijkstra_solver.h"
#include <algorithm>
#include <queue>
#include <unordered_map>

struct Node {
    Point point;
    int dist;

    bool operator>(const Node& other) const {
        return dist > other.dist;
    }
};

SolvingResult DijkstraSolver::solve(const Maze& maze) {
    SolvingResult result;
    result.found = false;

    int width = maze.getWidth();
    int height = maze.getHeight();

    Point start = maze.getStart();
    Point end = maze.getEnd();

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::unordered_map<int, int> parent;
    std::unordered_map<int, int> dist;

    auto pointToIndex = [width](const Point& p) {
        return p.y * width + p.x;
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    dist[pointToIndex(start)] = 0;
    pq.push({start, 0});

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

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
                    int newDist = current.dist + 1;
                    int idx = pointToIndex({nx, ny});

                    if (!visited[ny][nx] && (dist.find(idx) == dist.end() || newDist < dist[idx])) {
                        dist[idx] = newDist;
                        parent[idx] = pointToIndex(current.point);
                        pq.push({{nx, ny}, newDist});
                    }
                }
            }
        }
    }

    return result;
}
