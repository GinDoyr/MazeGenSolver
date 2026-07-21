#ifndef MAZE_WIDGET_H
#define MAZE_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <vector>
#include "maze.h"
#include "generator.h"

/* Путь, найденный одним алгоритмом решения, вместе с визуальными параметрами.
   Хранится в MazeWidget и используется для отрисовки и анимации. */
struct SolutionPath {
    std::vector<Point> path;              /* Финальный путь от старта до финиша */
    std::vector<Point> visitedOrder;      /* Все посещённые клетки в порядке посещения */
    QColor pathColor;                     /* Цвет линии пути */
    QColor visitedColor;                  /* Цвет уже посещённых клеток (полупрозрачный) */
    QColor frontierColor;                 /* Цвет клеток "в очереди" (фронт волны) */
    QString algorithmName;                /* Имя алгоритма — нужно для поиска в algorithmColors */
    double solveTimeMs;                   /* Время работы CPU в миллисекундах */
};

/* Виджет, который рисует лабиринт и анимирует работу алгоритмов.
   Отвечает за:
   - отрисовку стен, старта и финиша
   - анимацию генерации (пошаговое удаление стен)
   - анимацию решения (волны посещения клеток)
   - пользовательское прохождение (движение игрока)
   - управление камерой (зум колесом, перетаскивание мышью) */
class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget* parent = nullptr);

    /* Устанавливает лабиринт для отрисовки. Сбрасывает все состояния анимации. */
    void setMaze(const Maze& maze);

    /* Передаёт вектор шагов генерации для анимации. */
    void setGenerationSteps(const std::vector<GenerationStep>& steps);
    const std::vector<GenerationStep>& getGenerationSteps() const { return steps; }

    /* Задаёт задержку между кадрами анимации генерации в миллисекундах. */
    void setGenerationDelay(int delayMs);

    /* Запускает анимацию генерации с текущей задержкой. */
    void startGenerationAnimation();

    /* Останавливает анимацию генерации (таймер). */
    void stopAnimation();

    /* Возвращает камеру в исходное положение (вписывает лабиринт в виджет). */
    void resetView();

    /* Включает/выключает отрисовку линии текущего пути при генерации DFS. */
    void setPathVisible(bool visible) { showPath = visible; update(); }

    /* Начинает пользовательское прохождение: игрок появляется на старте. */
    void startSolving();

    /* Останавливает пользовательское прохождение. */
    void stopSolving();

    /* Пытается переместить игрока в указанном направлении.
       Возвращает false, если там стена или координаты невалидны. */
    bool movePlayer(Direction dir);

    bool isSolvingState() const { return isSolving; }
    const Maze* getMaze() const { return &currentMaze; }

    /* Устанавливает пути от алгоритмов решения для анимации. */
    void setSolutionPaths(const std::vector<SolutionPath>& paths);

    /* Запускает анимацию решения с текущей задержкой. */
    void startSolvingAnimation();

    bool isMazeSolvingActive() const { return isMazeSolving; }

    /* Останавливает анимацию решения. */
    void stopSolvingAnimation();

    /* Задаёт размер "фронта" — сколько последних посещённых клеток
       рисуются ярким цветом. 0 = все клетки рисуются как фронт. */
    void setFrontierSize(int size);

    /* Очищает путь пользователя и сбрасывает состояние прохождения. */
    void clearUserPath();

    /* Очищает все пути решения и останавливает анимацию. */
    void clearSolutionPaths();

    /* Задаёт задержку между кадрами анимации решения в миллисекундах. */
    void setAnimationDelay(int delayMs);

    /* Приостанавливает анимацию решения (таймер останавливается,
       состояние сохраняется). */
    void pauseVisualization();

    /* Возобновляет анимацию решения с той же позиции. */
    void resumeVisualization();

    /* Перематывает анимацию решения к указанному шагу. */
    void seekTo(int step);

    /* Возвращает true, если любая визуализация (генерация или решение) на паузе. */
    bool isPaused() const { return isPausedViz || isPausedGen; }

    int getTotalSteps() const;
    int getCurrentStep() const;

    /* Приостанавливает анимацию генерации. */
    void pauseGeneration();

    /* Возобновляет анимацию генерации. */
    void resumeGeneration();

    /* Перематывает анимацию генерации к указанному шагу.
       Восстанавливает состояние лабиринта и посещённых клеток. */
    void seekGenerationTo(int step);

    bool isGeneratingActive() const { return isGeneratingViz; }
    int getTotalGenerationSteps() const;
    int getCurrentGenerationStep() const;

    /* Обновляет цвет алгоритма в текущих путях решения.
       Пересчитывает visitedColor и frontierColor на основе нового основного. */
    void updateAlgorithmColor(const QString& algorithmName, const QColor& newColor);

signals:
    void generationFinished();
    void solvingFinished(int steps, double timeSeconds, int pathLength);
    void solvingStatsUpdated(int steps, double timeSeconds, int pathLength);
    void mazeSolvingFinished();
    void visualizationProgress(int current, int total);
    void generationVisualizationProgress(int current, int total);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onAnimationTick();
    void onSolvingTimerTick();
    void onMazeSolvingTick();

private:
    void drawMaze(QPainter& painter);
    void drawGenerationProgress(QPainter& painter);
    void drawSolvingProgress(QPainter& painter);
    void drawSolutionPaths(QPainter& painter);

    /* Вписывает лабиринт в текущий размер виджета с небольшим отступом (0.95). */
    void fitToView();

    /* Добавляет клетку в путь пользователя. Если клетка уже была в пути —
       обрезает путь до неё (откат при возврате на уже посещённую клетку). */
    void addToPath(const Point& p);

    /* Текущий лабиринт. Для анимации генерации сюда кладётся "чистая" копия,
       а стены убираются по шагам из steps. */
    Maze currentMaze;

    /* Шаги генерации для анимации. */
    std::vector<GenerationStep> steps;
    size_t currentStepIndex;
    QTimer animationTimer;

    /* Параметры камеры: масштаб и смещение. */
    double scale;
    double offsetX;
    double offsetY;
    double baseCellSize;

    /* Состояние перетаскивания мышью. */
    bool isDragging;
    QPoint lastMousePos;

    /* Рисовать ли линию текущего пути (стек DFS) при генерации. */
    bool showPath;

    /* Посещённые клетки и текущий путь для анимации генерации. */
    std::vector<std::vector<bool>> visitedCells;
    std::vector<std::pair<int, int>> currentPath;

    /* Состояние пользовательского прохождения. */
    bool isSolving;
    bool showUserPath;
    Point playerPosition;
    std::vector<Point> userPath;
    QElapsedTimer solveTimer;
    QTimer statsTimer;
    int stepCount;

    /* Пути от алгоритмов решения и индексы прогресса для каждого. */
    std::vector<SolutionPath> solutionPaths;
    std::vector<std::vector<bool>> solvedCells;
    std::vector<size_t> currentPathIndices;
    std::vector<size_t> currentVisitedIndices;

    int frontierSize;
    QTimer mazeSolvingTimer;
    bool isMazeSolving;

    /* Состояние анимации генерации. */
    bool isGeneratingViz;
    bool isPausedGen;
    int generationDelayMs;

    /* Состояние анимации решения. */
    bool isPausedViz;
    int animationDelayMs;
    int manualSeekPosition;
};

#endif