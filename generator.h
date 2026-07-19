#ifndef GENERATOR_H
#define GENERATOR_H

#include "maze.h"
#include <vector>
#include <functional>

struct GenerationStep {
    enum Type {
        VisitCell,
        RemoveWall,
        Backtrack
    };
    Type type;
    int x, y;
    Direction dir;
};

class Generator {
protected:
    double wallDensity;
    std::vector<GenerationStep> *stepsRecorder;  // Указатель на вектор для записи шагов

    void addExtraPassages(Maze& maze);
    void recordStep(GenerationStep::Type type, int x, int y, Direction dir);

public:
    Generator(double density = 1.0) : wallDensity(density), stepsRecorder(nullptr) {}
    virtual ~Generator() = default;
    virtual void generate(Maze& maze) = 0;

    void setStepsRecorder(std::vector<GenerationStep> *recorder) {
        stepsRecorder = recorder;
    }
};

#endif
