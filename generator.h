#ifndef GENERATOR_H
#define GENERATOR_H

#include "maze.h"
#include <vector>
#include <functional>

/**
 * @brief Один шаг алгоритма генерации.
 *
 * Хранится в векторе и передаётся в MazeWidget для пошаговой анимации.
 * Генератор сам решает, какие действия записывать — виджет только воспроизводит.
 */
struct GenerationStep {
    enum Type {
        VisitCell,    ///< Клетка помечена как посещённая
        RemoveWall,   ///< Удалена стена
        Backtrack     ///< Откат к предыдущей клетке
    };

    Type type;        ///< Что делаем
    int x, y;         ///< Координаты клетки
    Direction dir;    ///< Направление (нужно для RemoveWall)
};

/**
 * @brief Базовый класс для всех генераторов лабиринтов.
 *
 * Хранит плотность стен и указатель на вектор шагов.
 * Конкретные алгоритмы (DFS, Крускал, Прим) наследуются отсюда
 * и реализуют метод generate().
 *
 * Общая логика (добавление лишних проходов, запись шагов)
 * вынесена сюда, чтобы не дублировать её в каждом наследнике.
 */
class Generator {
protected:
    double wallDensity;
    std::vector<GenerationStep>* stepsRecorder;

    /**
     * @brief Случайно удаляет часть стен после основной генерации.
     *
     * Чем ниже wallDensity, тем больше стен уйдёт.
     * При плотности 1.0 лабиринт остаётся идеальным (без циклов).
     */
    void addExtraPassages(Maze& maze);

    /**
     * @brief Записывает шаг в вектор анимации.
     *
     * Если stepsRecorder == nullptr, вызов ничего не делает.
     * Это позволяет использовать генератор без накладных расходов,
     * когда анимация не нужна.
     */
    void recordStep(GenerationStep::Type type, int x, int y, Direction dir);

public:
    Generator(double density = 1.0) : wallDensity(density), stepsRecorder(nullptr) {}
    virtual ~Generator() = default;

    /** Запускает генерацию. Лабиринт должен быть уже создан с нужными размерами. */
    virtual void generate(Maze& maze) = 0;

    /** Подключает внешний вектор для записи шагов анимации. */
    void setStepsRecorder(std::vector<GenerationStep>* recorder) {
        stepsRecorder = recorder;
    }
};

#endif