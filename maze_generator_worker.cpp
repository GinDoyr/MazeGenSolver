#include "maze_generator_worker.h"
#include "generators/dfs_generator.h"
#include "generators/kruskal_generator.h"
#include "generators/prim_generator.h"

/* Создаёт нужный генератор по имени из UI и запускает его.
 Фабричный метод: MainWindow не знает о конкретных классах генераторов,
 работает только через базовый Generator. Это позволяет добавлять новые
 алгоритмы, не трогая код главного окна.
 */
void MazeGeneratorWorker::generate(Maze& maze, const QString& algorithmName,
    double density, std::vector<GenerationStep>& outSteps)
{
    outSteps.clear();

    std::unique_ptr<Generator> generator;

    if (algorithmName == QString::fromUtf8("DFS (Поиск в Глубину)")) {
        generator = std::make_unique<DFSGenerator>(density);
    }
    else if (algorithmName == QString::fromUtf8("Краскал")) {
        generator = std::make_unique<KruskalGenerator>(density);
    }
    else if (algorithmName == QString::fromUtf8("Прим")) {
        generator = std::make_unique<PrimGenerator>(density);
    }
    else {
        // Fallback — на случай, если в UI появится неизвестное значение
        generator = std::make_unique<DFSGenerator>(density);
    }

    // Подключаем вектор шагов — генератор будет писать в него по ходу работы
    generator->setStepsRecorder(&outSteps);
    generator->generate(maze);
}