#include "maze_generator_worker.h"
#include "generators/dfs_generator.h"
#include "generators/kruskal_generator.h"
#include "generators/prim_generator.h"

void MazeGeneratorWorker::generate(Maze &maze, const QString &algorithmName,
                                   double density, std::vector<GenerationStep> &outSteps)
{
    outSteps.clear();

    std::unique_ptr<IGenerator> generator;

    if (algorithmName == QString::fromUtf8("DFS (Поиск в Глубину)")) {
        generator = std::make_unique<DFSGenerator>(density);
    } else if (algorithmName == QString::fromUtf8("Краскал")) {
        generator = std::make_unique<KruskalGenerator>(density);
    } else if (algorithmName == QString::fromUtf8("Прим")) {
        generator = std::make_unique<PrimGenerator>(density);
    } else {
        generator = std::make_unique<DFSGenerator>(density);
    }

    generator->setStepsRecorder(&outSteps);
    generator->generate(maze);
}
