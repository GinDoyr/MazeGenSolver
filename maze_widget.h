#ifndef MAZE_WIDGET_H
#define MAZE_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <vector>
#include "maze.h"
#include "generator.h"

// Структура для хранения пути решения с цветом
struct SolutionPath {
    std::vector<Point> path;              // Финальный путь
    std::vector<Point> visitedOrder;      // Все посещённые клетки в порядке посещения
    QColor pathColor;                     // Цвет линии пути
    QColor visitedColor;                  // Цвет посещённых клеток
    QColor frontierColor;                 // Цвет клеток "в очереди" (фронт)
    QString algorithmName;
    double solveTimeMs;
};

class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget *parent = nullptr);

    void setMaze(const Maze &maze);
    void setGenerationSteps(const std::vector<GenerationStep> &steps);
    const std::vector<GenerationStep>& getGenerationSteps() const { return steps; }
    void setGenerationDelay(int delayMs);
    void startGenerationAnimation();
    void stopAnimation();
    void resetView();
    void setPathVisible(bool visible) { showPath = visible; update(); }

    void startSolving();
    void stopSolving();
    bool movePlayer(Direction dir);
    bool isSolvingState() const { return isSolving; }
    const Maze* getMaze() const { return &currentMaze; }

    void setSolutionPaths(const std::vector<SolutionPath> &paths);
    void startSolvingAnimation();
    bool isMazeSolvingActive() const { return isMazeSolving; }
    void stopSolvingAnimation();

    void setFrontierSize(int size);   // 0 = безлимит
    void clearUserPath();  // Очистка пути пользователя
    void clearSolutionPaths();

    void setAnimationDelay(int delayMs);  // Скорость в клетках/сек
    void pauseVisualization();
    void resumeVisualization();
    void seekTo(int step);                    // Перемотка к шагу
    bool isPaused() const { return isPausedViz || isPausedGen;}
    int getTotalSteps() const;
    int getCurrentStep() const;

    // Управление визуализацией генерации
    void pauseGeneration();
    void resumeGeneration();
    void seekGenerationTo(int step);
    bool isGeneratingActive() const { return isGeneratingViz; }
    int getTotalGenerationSteps() const;
    int getCurrentGenerationStep() const;

    void updateAlgorithmColor(const QString &algorithmName, const QColor &newColor);

signals:
    void generationFinished();
    void solvingFinished(int steps, double timeSeconds, int pathLength);
    void solvingStatsUpdated(int steps, double timeSeconds, int pathLength);
    void mazeSolvingFinished();
    void visualizationProgress(int current, int total);
    void generationVisualizationProgress(int current, int total);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onAnimationTick();
    void onSolvingTimerTick();
    void onMazeSolvingTick();

private:
    void drawMaze(QPainter &painter);
    void drawGenerationProgress(QPainter &painter);
    void drawSolvingProgress(QPainter &painter);
    void drawSolutionPaths(QPainter &painter);
    void fitToView();
    void addToPath(const Point &p);

    Maze currentMaze;
    std::vector<GenerationStep> steps;
    size_t currentStepIndex;

    QTimer animationTimer;

    double scale;
    double offsetX;
    double offsetY;
    double baseCellSize;

    bool isDragging;
    QPoint lastMousePos;
    bool showPath;

    std::vector<std::vector<bool>> visitedCells;
    std::vector<std::pair<int, int>> currentPath;

    bool isSolving;
    bool showUserPath;
    Point playerPosition;
    std::vector<Point> userPath;
    QElapsedTimer solveTimer;
    QTimer statsTimer;
    int stepCount;

    std::vector<SolutionPath> solutionPaths;
    std::vector<std::vector<bool>> solvedCells;  // Для анимации
    std::vector<size_t> currentPathIndices;  // Текущий индекс для каждого пути
    std::vector<size_t> currentVisitedIndices;  // Текущий индекс посещённых для каждого пути
    int frontierSize;
    QTimer mazeSolvingTimer;
    bool isMazeSolving;

    bool isGeneratingViz;
    bool isPausedGen;
    int generationDelayMs;

    bool isPausedViz;        // Флаг паузы
    int animationDelayMs;      // Скорость анимации
    int manualSeekPosition;  // Позиция ручной перемотки (-1 = нет)
};

#endif
