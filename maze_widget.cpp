#include "maze_widget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <algorithm>
#include <cmath>

MazeWidget::MazeWidget(QWidget *parent)
    : QWidget(parent)
    , currentStepIndex(0)
    , scale(1.0)
    , offsetX(0.0)
    , offsetY(0.0)
    , baseCellSize(20.0)
    , isDragging(false)
    , showPath(true)
    , isSolving(false)
    , showUserPath(false)
    , stepCount(0)
    , isMazeSolving(false)
    , frontierSize(10)
    , isPausedViz(false)
    , animationDelayMs(0)
    , manualSeekPosition(-1)
    , isGeneratingViz(false)
    , isPausedGen(false)
    , generationDelayMs(0)
{
    setMinimumSize(400, 400);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    connect(&animationTimer, &QTimer::timeout, this, &MazeWidget::onAnimationTick);
    connect(&statsTimer, &QTimer::timeout, this, &MazeWidget::onSolvingTimerTick);
    connect(&mazeSolvingTimer, &QTimer::timeout, this, &MazeWidget::onMazeSolvingTick);
}

void MazeWidget::setMaze(const Maze &maze)
{
    currentMaze = maze;
    visitedCells.assign(currentMaze.getHeight(),
                        std::vector<bool>(currentMaze.getWidth(), false));
    currentPath.clear();
    currentStepIndex = 0;
    userPath.clear();
    showUserPath = false;
    solutionPaths.clear();
    isMazeSolving = false;
    fitToView();
    update();
}

void MazeWidget::setGenerationDelay(int delayMs)
{
    if (delayMs < 0) delayMs = 0;
    generationDelayMs = delayMs;

    if (animationTimer.isActive()) {
        animationTimer.setInterval(delayMs);
    }
}

void MazeWidget::setGenerationSteps(const std::vector<GenerationStep> &newSteps)
{
    steps = newSteps;
    currentStepIndex = 0;
    visitedCells.assign(currentMaze.getHeight(),
                        std::vector<bool>(currentMaze.getWidth(), false));
    currentPath.clear();
}

void MazeWidget::startGenerationAnimation()
{
    if (steps.empty()) {
        return;
    }
    currentStepIndex = 0;
    visitedCells.assign(currentMaze.getHeight(),
                        std::vector<bool>(currentMaze.getWidth(), false));
    currentPath.clear();

    isGeneratingViz = true;
    isPausedGen = false;

    animationTimer.setInterval(generationDelayMs);
    animationTimer.start();
}

void MazeWidget::stopAnimation()
{
    animationTimer.stop();
}

void MazeWidget::resetView()
{
    scale = 1.0;
    offsetX = 0.0;
    offsetY = 0.0;
    fitToView();
    update();
}

void MazeWidget::fitToView()
{
    if (currentMaze.getWidth() == 0 || currentMaze.getHeight() == 0) {
        return;
    }
    double mazeWidthPx = currentMaze.getWidth() * baseCellSize;
    double mazeHeightPx = currentMaze.getHeight() * baseCellSize;

    double scaleX = static_cast<double>(width()) / mazeWidthPx;
    double scaleY = static_cast<double>(height()) / mazeHeightPx;
    scale = std::min(scaleX, scaleY) * 0.95;

    double scaledWidth = mazeWidthPx * scale;
    double scaledHeight = mazeHeightPx * scale;
    offsetX = (width() - scaledWidth) / 2.0;
    offsetY = (height() - scaledHeight) / 2.0;
}

void MazeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(40, 40, 45));

    if (currentMaze.getWidth() == 0 || currentMaze.getHeight() == 0) {
        painter.setPen(QColor(150, 150, 150));
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
                         ("Нажмите \"Начать генерацию\""));
        return;
    }

    drawMaze(painter);

    // Приоритет: решение алгоритмами > прохождение пользователем > генерация
    if (isMazeSolving || !solutionPaths.empty()) {
        drawSolutionPaths(painter);
    } else if (isSolving || showUserPath) {
        drawSolvingProgress(painter);
    } else {
        drawGenerationProgress(painter);
    }
}

void MazeWidget::drawMaze(QPainter &painter)
{
    int w = currentMaze.getWidth();
    int h = currentMaze.getHeight();

    painter.save();
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    // Фон лабиринта
    painter.fillRect(0, 0, w * baseCellSize, h * baseCellSize,
                     QColor(245, 245, 245));

    // Стены
    QPen wallPen(QColor(30, 30, 30), 2.0 / scale);
    painter.setPen(wallPen);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double px = x * baseCellSize;
            double py = y * baseCellSize;

            if (currentMaze.hasWall(x, y, Direction::North)) {
                painter.drawLine(QPointF(px, py),
                                 QPointF(px + baseCellSize, py));
            }
            if (currentMaze.hasWall(x, y, Direction::West)) {
                painter.drawLine(QPointF(px, py),
                                 QPointF(px, py + baseCellSize));
            }
            if (x == w - 1 && currentMaze.hasWall(x, y, Direction::East)) {
                painter.drawLine(QPointF(px + baseCellSize, py),
                                 QPointF(px + baseCellSize, py + baseCellSize));
            }
            if (y == h - 1 && currentMaze.hasWall(x, y, Direction::South)) {
                painter.drawLine(QPointF(px, py + baseCellSize),
                                 QPointF(px + baseCellSize, py + baseCellSize));
            }
        }
    }

    // Старт и финиш
    Point start = currentMaze.getStart();
    Point end = currentMaze.getEnd();

    painter.fillRect(start.x * baseCellSize + 2,
                     start.y * baseCellSize + 2,
                     baseCellSize - 4, baseCellSize - 4,
                     QColor(76, 175, 80));

    painter.fillRect(end.x * baseCellSize + 2,
                     end.y * baseCellSize + 2,
                     baseCellSize - 4, baseCellSize - 4,
                     QColor(244, 67, 54));

    painter.restore();
}

void MazeWidget::drawGenerationProgress(QPainter &painter)
{
    if (visitedCells.empty()) {
        return;
    }

    int w = currentMaze.getWidth();
    int h = currentMaze.getHeight();

    painter.save();
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    // Посещённые клетки
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (visitedCells[y][x]) {
                painter.fillRect(x * baseCellSize + 3,
                                 y * baseCellSize + 3,
                                 baseCellSize - 6, baseCellSize - 6,
                                 QColor(100, 181, 246, 180));
            }
        }
    }

    // Текущий путь (стек в DFS)
    if (showPath) {
        painter.setPen(QPen(QColor(33, 150, 243), 2.0 / scale));
        for (size_t i = 1; i < currentPath.size(); ++i) {
            double x1 = currentPath[i - 1].first * baseCellSize + baseCellSize / 2.0;
            double y1 = currentPath[i - 1].second * baseCellSize + baseCellSize / 2.0;
            double x2 = currentPath[i].first * baseCellSize + baseCellSize / 2.0;
            double y2 = currentPath[i].second * baseCellSize + baseCellSize / 2.0;
            painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }
    }

    painter.restore();
}

void MazeWidget::onAnimationTick()
{
    if (!isGeneratingViz || isPausedGen) {
        return;
    }

    // Пропускаем шаги Backtrack
    while (currentStepIndex < steps.size() &&
           steps[currentStepIndex].type == GenerationStep::Backtrack) {
        ++currentStepIndex;
    }

    if (currentStepIndex >= steps.size()) {
        animationTimer.stop();
        isGeneratingViz = false;
        emit generationFinished();
        emit generationVisualizationProgress(getTotalGenerationSteps(), getTotalGenerationSteps());
        return;
    }

    const GenerationStep &step = steps[currentStepIndex];

    switch (step.type) {
    case GenerationStep::VisitCell:
        visitedCells[step.y][step.x] = true;
        if (showPath) {
            currentPath.push_back({step.x, step.y});
        }
        break;
    case GenerationStep::RemoveWall:
        currentMaze.removeWall(step.x, step.y, step.dir);
        break;
    case GenerationStep::Backtrack:
        break;
    }

    ++currentStepIndex;
    update();
    emit generationVisualizationProgress(getCurrentGenerationStep(), getTotalGenerationSteps());
}

void MazeWidget::wheelEvent(QWheelEvent *event)
{
    double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;

    QPoint mousePos = event->position().toPoint();
    double mx = mousePos.x();
    double my = mousePos.y();

    double newScale = scale * factor;
    newScale = std::max(0.1, std::min(newScale, 20.0));

    offsetX = mx - (mx - offsetX) * (newScale / scale);
    offsetY = my - (my - offsetY) * (newScale / scale);
    scale = newScale;

    update();
}

void MazeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        isDragging = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void MazeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging) {
        QPoint delta = event->pos() - lastMousePos;
        offsetX += delta.x();
        offsetY += delta.y();
        lastMousePos = event->pos();
        update();
    }
}

void MazeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void MazeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    fitToView();
}

void MazeWidget::startSolving()
{
    if (currentMaze.getWidth() == 0 || currentMaze.getHeight() == 0) {
        return;
    }

    isSolving = true;
    showUserPath = false;
    playerPosition = currentMaze.getStart();
    userPath.clear();
    userPath.push_back(playerPosition);
    stepCount = 0;

    // Очищаем клетки генерации
    visitedCells.assign(currentMaze.getHeight(),
                        std::vector<bool>(currentMaze.getWidth(), false));
    currentPath.clear();

    solveTimer.start();
    statsTimer.start(100);  // Обновление статистики каждые 100 мс

    update();
}

void MazeWidget::stopSolving()
{
    isSolving = false;
    statsTimer.stop();
    update();
}

bool MazeWidget::movePlayer(Direction dir)
{
    if (!isSolving) {
        return false;
    }

    if (currentMaze.hasWall(playerPosition.x, playerPosition.y, dir)) {
        return false;
    }

    int nx = playerPosition.x;
    int ny = playerPosition.y;

    switch (dir) {
    case Direction::North: ny--; break;
    case Direction::South: ny++; break;
    case Direction::East:  nx++; break;
    case Direction::West:  nx--; break;
    }

    if (!currentMaze.isValid(nx, ny)) {
        return false;
    }

    playerPosition = {nx, ny};
    stepCount++;
    addToPath(playerPosition);

    if (playerPosition == currentMaze.getEnd()) {
        double elapsed = solveTimer.elapsed() / 1000.0;
        statsTimer.stop();
        isSolving = false;
        showUserPath = true;
        emit solvingFinished(stepCount, elapsed, static_cast<int>(userPath.size()));
    } else {
        double elapsed = solveTimer.elapsed() / 1000.0;
        emit solvingStatsUpdated(stepCount, elapsed, static_cast<int>(userPath.size()));
    }

    update();
    return true;
}

void MazeWidget::addToPath(const Point &p)
{
    // Проверяем, есть ли эта точка уже в пути
    for (size_t i = 0; i < userPath.size(); ++i) {
        if (userPath[i] == p) {
            // Удаляем эту точку и все последующие (откат)
            userPath.resize(i + 1);
            return;
        }
    }
    // Если точки нет в пути - добавляем
    userPath.push_back(p);
}

void MazeWidget::onSolvingTimerTick()
{
    if (isSolving) {
        double elapsed = solveTimer.elapsed() / 1000.0;
        emit solvingStatsUpdated(stepCount, elapsed, static_cast<int>(userPath.size()));
    }
}

void MazeWidget::drawSolvingProgress(QPainter &painter)
{
    if (!isSolving && !showUserPath) {
        return;
    }

    painter.save();
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    // Рисуем путь пользователя ТОЛЬКО после завершения прохождения
    if (showUserPath && userPath.size() > 1) {
        painter.setPen(QPen(QColor(255, 152, 0), 3.0 / scale));
        for (size_t i = 1; i < userPath.size(); ++i) {
            double x1 = userPath[i - 1].x * baseCellSize + baseCellSize / 2.0;
            double y1 = userPath[i - 1].y * baseCellSize + baseCellSize / 2.0;
            double x2 = userPath[i].x * baseCellSize + baseCellSize / 2.0;
            double y2 = userPath[i].y * baseCellSize + baseCellSize / 2.0;
            painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }
    }

    double px = playerPosition.x * baseCellSize + baseCellSize * 0.15;
    double py = playerPosition.y * baseCellSize + baseCellSize * 0.15;
    double size = baseCellSize * 0.7;

    painter.setBrush(QColor(255, 235, 59));
    painter.setPen(QPen(QColor(245, 127, 23), 2.0 / scale));
    painter.drawEllipse(QPointF(px + size / 2.0, py + size / 2.0),
                        size / 2.0, size / 2.0);

    painter.restore();
}

void MazeWidget::setSolutionPaths(const std::vector<SolutionPath> &paths)
{
    solutionPaths = paths;
    solvedCells.clear();
    currentPathIndices.clear();
    currentVisitedIndices.clear();

    // Инициализируем структуры для анимации
    for (size_t i = 0; i < solutionPaths.size(); ++i) {
        solvedCells.push_back(std::vector<bool>(
            currentMaze.getHeight() * currentMaze.getWidth(), false));
        currentPathIndices.push_back(0);
        currentVisitedIndices.push_back(0);
    }
}

void MazeWidget::startSolvingAnimation()
{
    if (solutionPaths.empty()) {
        return;
    }

    isMazeSolving = true;
    isPausedViz = false;
    manualSeekPosition = -1;

    mazeSolvingTimer.setInterval(animationDelayMs);
    mazeSolvingTimer.start();
}

void MazeWidget::stopSolvingAnimation()
{
    isMazeSolving = false;
    mazeSolvingTimer.stop();
}

void MazeWidget::onMazeSolvingTick()
{
    if (!isMazeSolving || isPausedViz) {
        return;
    }

    bool allFinished = true;

    for (size_t i = 0; i < solutionPaths.size(); ++i) {
        const SolutionPath &solPath = solutionPaths[i];

        if (currentVisitedIndices[i] < solPath.visitedOrder.size()) {
            const Point &p = solPath.visitedOrder[currentVisitedIndices[i]];
            int index = p.y * currentMaze.getWidth() + p.x;
            solvedCells[i][index] = true;
            currentVisitedIndices[i]++;
            allFinished = false;
        }
    }

    update();

    int currentStep = getCurrentStep();
    int totalSteps = getTotalSteps();
    emit visualizationProgress(currentStep, totalSteps);

    if (allFinished) {
        mazeSolvingTimer.stop();
        isMazeSolving = false;
        emit mazeSolvingFinished();
    }
}

void MazeWidget::drawSolutionPaths(QPainter &painter)
{
    if (solutionPaths.empty()) {
        return;
    }

    painter.save();
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    for (size_t i = 0; i < solutionPaths.size(); ++i) {
        const SolutionPath &solPath = solutionPaths[i];
        size_t visitedIdx = currentVisitedIndices[i];

        // 1. Рисуем посещённые клетки (полупрозрачный фон)
        for (size_t j = 0; j < visitedIdx && j < solPath.visitedOrder.size(); ++j) {
            const Point &p = solPath.visitedOrder[j];

            // Определяем, является ли клетка "фронтом"
            bool isFrontier;// Только последние frontierSize клеток - фронт
            isFrontier = (j + frontierSize >= visitedIdx) && (j < visitedIdx);

            QColor cellColor = isFrontier ? solPath.frontierColor : solPath.visitedColor;

            painter.fillRect(p.x * baseCellSize + 2,
                             p.y * baseCellSize + 2,
                             baseCellSize - 4, baseCellSize - 4,
                             cellColor);
        }

        // 2. Рисуем линию финального пути ТОЛЬКО по посещённым клеткам
        // Находим, до какой клетки пути все клетки уже посещены
        size_t pathDrawEnd = 0;
        for (size_t j = 0; j < solPath.path.size(); ++j) {
            const Point &p = solPath.path[j];
            int index = p.y * currentMaze.getWidth() + p.x;
            if (solvedCells[i][index]) {
                pathDrawEnd = j + 1;  // Можно рисовать до этой клетки
            } else {
                break;  // Останавливаемся на первой непосещённой клетке
            }
        }

        if (pathDrawEnd > 1) {
            painter.setPen(QPen(solPath.pathColor, 3.0 / scale));
            for (size_t j = 1; j < pathDrawEnd; ++j) {
                double x1 = solPath.path[j - 1].x * baseCellSize + baseCellSize / 2.0;
                double y1 = solPath.path[j - 1].y * baseCellSize + baseCellSize / 2.0;
                double x2 = solPath.path[j].x * baseCellSize + baseCellSize / 2.0;
                double y2 = solPath.path[j].y * baseCellSize + baseCellSize / 2.0;
                painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
            }
        }
    }

    painter.restore();
}

void MazeWidget::setFrontierSize(int size)
{
    frontierSize = (size < 0) ? 0 : size;
    update();
}

void MazeWidget::clearUserPath()
{
    userPath.clear();
    showUserPath = false;
    isSolving = false;
    statsTimer.stop();
    update();
}

void MazeWidget::clearSolutionPaths()
{
    solutionPaths.clear();
    solvedCells.clear();
    currentPathIndices.clear();
    currentVisitedIndices.clear();
    isMazeSolving = false;
    isPausedViz = false;
    mazeSolvingTimer.stop();
    update();
}

void MazeWidget::setAnimationDelay(int delayMs)
{
    if (delayMs < 0) delayMs = 0;
    animationDelayMs = delayMs;

    if (mazeSolvingTimer.isActive()) {
        mazeSolvingTimer.setInterval(delayMs);
    }
}

void MazeWidget::pauseVisualization()
{
    if (isMazeSolving) {
        isPausedViz = true;
        mazeSolvingTimer.stop();
    }
}

void MazeWidget::resumeVisualization()
{
    if (isMazeSolving && isPausedViz) {
        isPausedViz = false;
        mazeSolvingTimer.setInterval(animationDelayMs);
        mazeSolvingTimer.start();
    }
}

int MazeWidget::getTotalSteps() const
{
    if (solutionPaths.empty()) return 0;

    size_t maxSteps = 0;
    for (const auto &path : solutionPaths) {
        if (path.visitedOrder.size() > maxSteps) {
            maxSteps = path.visitedOrder.size();
        }
    }
    return static_cast<int>(maxSteps);
}

int MazeWidget::getCurrentStep() const
{
    if (currentVisitedIndices.empty()) return 0;

    size_t maxStep = 0;
    for (size_t idx : currentVisitedIndices) {
        if (idx > maxStep) maxStep = idx;
    }
    return static_cast<int>(maxStep);
}

void MazeWidget::seekTo(int step)
{
    if (solutionPaths.empty()) return;

    manualSeekPosition = step;

    // Сбрасываем все solvedCells
    for (size_t i = 0; i < solutionPaths.size(); ++i) {
        std::fill(solvedCells[i].begin(), solvedCells[i].end(), false);
    }

    // Восстанавливаем состояние до указанного шага
    for (size_t i = 0; i < solutionPaths.size(); ++i) {
        const SolutionPath &solPath = solutionPaths[i];
        size_t targetIdx = static_cast<size_t>(
            std::min(step, static_cast<int>(solPath.visitedOrder.size())));

        for (size_t j = 0; j < targetIdx; ++j) {
            const Point &p = solPath.visitedOrder[j];
            int index = p.y * currentMaze.getWidth() + p.x;
            solvedCells[i][index] = true;
        }
        currentVisitedIndices[i] = targetIdx;
    }

    update();
    emit visualizationProgress(getCurrentStep(), getTotalSteps());
}

void MazeWidget::updateAlgorithmColor(const QString &algorithmName, const QColor &newColor)
{
    for (auto &path : solutionPaths) {
        if (path.algorithmName == algorithmName) {
            path.pathColor = newColor;

            // visitedColor — полупрозрачная версия основного цвета
            path.visitedColor = QColor(newColor.red(), newColor.green(), newColor.blue(), 90);

            // frontierColor — яркая и насыщенная версия
            QColor hsl = newColor.toHsl();
            int newLightness = std::min(255, static_cast<int>(hsl.lightness() * 1.4));
            int newSaturation = std::min(255, static_cast<int>(hsl.saturation() * 1.3));
            path.frontierColor = QColor::fromHsl(hsl.hue(), newSaturation, newLightness, 230);

            break;
        }
    }
    update();
}

void MazeWidget::pauseGeneration()
{
    if (isGeneratingViz) {
        isPausedGen = true;
        animationTimer.stop();
    }
}

void MazeWidget::resumeGeneration()
{
    if (isGeneratingViz && isPausedGen) {
        isPausedGen = false;
        animationTimer.setInterval(generationDelayMs);
        animationTimer.start();
    }
}

int MazeWidget::getTotalGenerationSteps() const
{
    return static_cast<int>(steps.size());
}

int MazeWidget::getCurrentGenerationStep() const
{
    return static_cast<int>(currentStepIndex);
}

void MazeWidget::seekGenerationTo(int step)
{
    if (steps.empty()) return;

    // Сбрасываем состояние
    visitedCells.assign(currentMaze.getHeight(),
                        std::vector<bool>(currentMaze.getWidth(), false));
    currentPath.clear();

    // Восстанавливаем состояние до указанного шага
    size_t targetStep = static_cast<size_t>(
        std::min(step, static_cast<int>(steps.size())));

    // Создаём временную копию лабиринта для применения шагов
    Maze tempMaze = currentMaze.createEmptyCopy();

    for (size_t i = 0; i < targetStep; ++i) {
        const GenerationStep &s = steps[i];
        switch (s.type) {
        case GenerationStep::VisitCell:
            visitedCells[s.y][s.x] = true;
            currentPath.push_back({s.x, s.y});
            break;
        case GenerationStep::RemoveWall:
            tempMaze.removeWall(s.x, s.y, s.dir);
            break;
        case GenerationStep::Backtrack:
            if (!currentPath.empty()) {
                currentPath.pop_back();
            }
            break;
        }
    }

    // Применяем все удаления стен к текущему лабиринту
    for (int y = 0; y < currentMaze.getHeight(); ++y) {
        for (int x = 0; x < currentMaze.getWidth(); ++x) {
            for (int d = 0; d < 4; ++d) {
                Direction dir = static_cast<Direction>(d);
                if (!tempMaze.hasWall(x, y, dir) && currentMaze.hasWall(x, y, dir)) {
                    currentMaze.removeWall(x, y, dir);
                }
            }
        }
    }

    currentStepIndex = targetStep;
    update();
    emit generationVisualizationProgress(getCurrentGenerationStep(), getTotalGenerationSteps());
}
