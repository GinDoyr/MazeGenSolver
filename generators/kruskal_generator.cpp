#include "kruskal_generator.h"
#include <algorithm>

KruskalGenerator::KruskalGenerator(double density, unsigned int seed)
    : Generator(density) {
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    }
    else {
        rng.seed(seed);
    }
}

int KruskalGenerator::find(int i) {
    if (parent[i] != i) {
        parent[i] = find(parent[i]);
    }
    return parent[i];
}

void KruskalGenerator::unite(int i, int j) {
    int rootI = find(i);
    int rootJ = find(j);

    if (rootI != rootJ) {
        if (rank[rootI] < rank[rootJ]) {
            parent[rootI] = rootJ;
        }
        else if (rank[rootI] > rank[rootJ]) {
            parent[rootJ] = rootI;
        }
        else {
            parent[rootJ] = rootI;
            rank[rootI]++;
        }
    }
}

void KruskalGenerator::generate(Maze& maze) {
    int width = maze.getWidth();
    int height = maze.getHeight();

    int totalCells = width * height;
    parent.resize(totalCells);
    rank.resize(totalCells, 0);

    for (int i = 0; i < totalCells; ++i) {
        parent[i] = i;
    }

    std::vector<Edge> edges;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x + 1 < width) {
                edges.push_back({ x, y, x + 1, y, Direction::East });
            }
            if (y + 1 < height) {
                edges.push_back({ x, y, x, y + 1, Direction::South });
            }
        }
    }

    std::shuffle(edges.begin(), edges.end(), rng);

    for (const Edge& edge : edges) {
        int cell1 = edge.y1 * width + edge.x1;
        int cell2 = edge.y2 * width + edge.x2;

        if (find(cell1) != find(cell2)) {
            maze.removeWall(edge.x1, edge.y1, edge.dir);
            recordStep(GenerationStep::RemoveWall, edge.x1, edge.y1, edge.dir);
            recordStep(GenerationStep::VisitCell, edge.x2, edge.y2, Direction::North);
            unite(cell1, cell2);
        }
    }

    addExtraPassages(maze);
}
