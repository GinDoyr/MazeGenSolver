#include "maze.h"

Maze::Maze()
    : width(0), height(0)
    , start{ 0, 0 }
    , end{ 0, 0 } {
}

Maze::Maze(int width, int height)
    : width(width), height(height)
    , start{ 0, 0 }
    , end{ width - 1, height - 1 } {
    cells.resize(height, std::vector<Cell>(width));
}

Cell& Maze::getCell(int x, int y) {
    return cells[y][x];
}

const Cell& Maze::getCell(int x, int y) const {
    return cells[y][x];
}

bool Maze::isValid(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool Maze::isValidPoint(const Point& p) const {
    return isValid(p.x, p.y);
}

bool Maze::hasWall(int x, int y, Direction dir) const {
    return cells[y][x].walls[static_cast<int>(dir)];
}

void Maze::removeWall(int x, int y, Direction dir) {
    int dirInt = static_cast<int>(dir);
    cells[y][x].walls[dirInt] = false;

    // Вычисляем координаты соседней клетки
    int nx = x, ny = y;
    switch (dir) {
    case Direction::North: ny--; break;
    case Direction::South: ny++; break;
    case Direction::East:  nx++; break;
    case Direction::West:  nx--; break;
    }

    // Убираем противоположную стену у соседа, если он в пределах лабиринта
    if (isValid(nx, ny)) {
        int oppositeDir = (dirInt + 2) % 4;
        cells[ny][nx].walls[oppositeDir] = false;
    }
}

void Maze::resetVisited() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            cells[y][x].visited = false;
        }
    }
}

void Maze::setStart(const Point& p) {
    if (isValidPoint(p) && p != end) {
        start = p;
    }
    else {
        start = { 0, 0 };
    }
}

void Maze::setEnd(const Point& p) {
    if (isValidPoint(p) && p != start) {
        end = p;
    }
    else {
        end = { width - 1, height - 1 };
    }
}

void Maze::setStartAndEnd(const Point& newStart, const Point& newEnd) {
    bool startValid = isValidPoint(newStart);
    bool endValid = isValidPoint(newEnd);
    bool notEqual = newStart != newEnd;

    if (startValid && endValid && notEqual) {
        start = newStart;
        end = newEnd;
    }
    else {
        start = { 0, 0 };
        end = { width - 1, height - 1 };
    }
}

Maze Maze::createEmptyCopy() const
{
    Maze copy(width, height);
    copy.start = start;
    copy.end = end;
    // Стены у copy уже установлены в true конструктором Cell
    return copy;
}