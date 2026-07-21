#ifndef MAZE_GENERATOR_WORKER_H
#define MAZE_GENERATOR_WORKER_H

#include "maze.h"
#include "maze_widget.h"
#include <vector>
#include <memory>

/* Фабрика генераторов.
   По названию алгоритма из UI создаёт нужный объект-наследник Generator
   и запускает его. MainWindow не знает о конкретных классах генераторов —
   работает только через базовый Generator. */
class MazeGeneratorWorker
{
public:
    /* Запускает выбранные алгоритмы решения и собирает их результаты.
       @param maze Лабиринт, который будет заполнен алгоритмом
       @param algorithmName Название из comb_alggen
       @param density Плотность стен [0.0; 1.0]
       @param outSteps Вектор, в который будут записаны шаги для анимации */
    static void generate(Maze& maze, const QString& algorithmName,
        double density, std::vector<GenerationStep>& outSteps);
};

#endif