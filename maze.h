#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <cstdint>

/**
 * @brief Направления движения в лабиринте.
 *
 * Порядок важен: противоположные направления отличаются на 2
 * (North<->South, East<->West). Это используется в removeWall()
 * для симметричного удаления стен у соседних клеток.
 */
enum class Direction {
    North = 0,
    East = 1,
    South = 2,
    West = 3
};

/**
 * @brief Координаты клетки в лабиринте.
 *
 * x — столбец (горизонталь), y — строка (вертикаль).
 * Начало координат в левом верхнем углу.
 */
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

/**
 * @brief Одна клетка лабиринта.
 *
 * Хранит четыре стены (N, E, S, W) и флаг посещённости.
 * По умолчанию все стены установлены — лабиринт начинается
 * как замкнутое пространство, алгоритмы генерации их убирают.
 */
struct Cell {
    bool walls[4];  // Индексы соответствуют Direction
    bool visited;

    Cell() : visited(false) {
        walls[0] = walls[1] = walls[2] = walls[3] = true;
    }
};

/**
 * @brief Модель лабиринта.
 *
 * Хранит двумерный массив клеток, координаты старта и финиша.
 * Не содержит логики генерации или решения — только структуру
 * и базовые операции (проверка стен, валидация координат).
 *
 * Алгоритмы работают с Maze через публичные методы, не обращаясь
 * к cells напрямую. Это позволяет менять внутреннее представление
 * без переписывания генераторов и решателей.
 */
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

    /* Проверяет, находятся ли координаты внутри лабиринта */
    bool isValid(int x, int y) const;

    /* Проверяет наличие стены у клетки в указанном направлении */
    bool hasWall(int x, int y, Direction dir) const;

    /**
     * @brief Удаляет стену между двумя клетками.
     *
     * Симметрично убирает стену и у соседней клетки:
     * если убираем East у (x,y), то убираем West у (x+1,y).
     * Это нужно, чтобы отрисовка и алгоритмы видели один и тот же лабиринт.
     */
    void removeWall(int x, int y, Direction dir);

    /** Сбрасывает флаги visited у всех клеток (перед новым запуском алгоритма) */
    void resetVisited();

    Point getStart() const { return start; }
    Point getEnd() const { return end; }

    void setStart(const Point& p);
    void setEnd(const Point& p);

    /**
     * @brief Устанавливает старт и финиш одновременно.
     *
     * Если координаты некорректны (за пределами, совпадают) —
     * сбрасывает к значениям по умолчанию: (0,0) и (w-1, h-1).
     */
    void setStartAndEnd(const Point& start, const Point& end);

    /**
     * @brief Создаёт копию лабиринта со всеми стенами на месте.
     *
     * Используется для анимации генерации: генератор модифицирует
     * исходный Maze, а виджет получает "чистую" копию и постепенно
     * убирает стены по шагам из вектора GenerationStep.
     */
    Maze createEmptyCopy() const;
};

#endif