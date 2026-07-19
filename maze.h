#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <cstdint>

enum class Direction {
    North = 0,
    East = 1,
    South = 2,
    West = 3
};

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

struct Cell {
    bool walls[4];  // N, E, S, W
    bool visited;

    Cell() : visited(false) {
        walls[0] = walls[1] = walls[2] = walls[3] = true;
    }
};

class Maze {
private:
    int width;
    int height;
    std::vector<std::vector<Cell>> cells;
    Point start;
    Point end;

    bool isValidPoint(const Point& p) const;

public:
    Maze();
    Maze(int width, int height);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Cell& getCell(int x, int y);
    const Cell& getCell(int x, int y) const;

    bool isValid(int x, int y) const;
    bool hasWall(int x, int y, Direction dir) const;
    void removeWall(int x, int y, Direction dir);

    void resetVisited();

    Point getStart() const { return start; }
    Point getEnd() const { return end; }

    void setStart(const Point& p);
    void setEnd(const Point& p);
    void setStartAndEnd(const Point& start, const Point& end);

    Maze createEmptyCopy() const;
};

#endif
