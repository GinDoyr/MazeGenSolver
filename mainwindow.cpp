#include "mainwindow.h"
#include "solvers/astar_solver.h"
#include "solvers/bfs_solver.h"
#include "solvers/dijkstra_solver.h"
#include "ui_mainwindow.h"
#include "maze_generator_worker.h"
#include <QStatusBar>
#include <QTimer>
#include <QMap>
#include <QColorDialog>
#include <QHeaderView>
#include <QElapsedTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isGenerating(false)
    , isSolving(false)
    , isMazeSolving(false)
    , currentSequentialIndex(0)
    , isParallelSolving(false)
    , isWaitingForNextAlgorithm(false)
    , currentFilter(0)
    , lastVizType(LastVisualization::None)
{
    ui->setupUi(this);

    window = NULL;
    infowindow = NULL;

    // Создаём виджет лабиринта и кладём в вертикальный layout
    mazeWidget = new MazeWidget(this);
    ui->verticalLayout_maze->addWidget(mazeWidget);

    // Подключаем сигналы от виджета к слотам окна
    connect(mazeWidget, &MazeWidget::generationFinished,
        this, &MainWindow::onGenerationFinished);
    connect(mazeWidget, &MazeWidget::solvingFinished,
        this, &MainWindow::onSolvingFinished);
    connect(mazeWidget, &MazeWidget::solvingStatsUpdated,
        this, &MainWindow::onSolvingStatsUpdated);
    connect(mazeWidget, &MazeWidget::mazeSolvingFinished,
        this, &MainWindow::onMazeSolvingFinished);
    connect(mazeWidget, &MazeWidget::visualizationProgress,
        this, &MainWindow::onVisualizationProgress);
    connect(mazeWidget, &MazeWidget::generationVisualizationProgress,
        this, &MainWindow::onGenerationVisualizationProgress);

    updateCoordinateRanges();

    // При изменении размеров лабиринта пересчитываем допустимые координаты
    connect(ui->spin_width, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int) { updateCoordinateRanges(); });
    connect(ui->spin_height, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int) { updateCoordinateRanges(); });

    connect(ui->check_ast, &QCheckBox::toggled, this, &MainWindow::onAlgorithmSelectionChanged);
    connect(ui->check_bfs, &QCheckBox::toggled, this, &MainWindow::onAlgorithmSelectionChanged);
    connect(ui->check_dej, &QCheckBox::toggled, this, &MainWindow::onAlgorithmSelectionChanged);

    // Цвета алгоритмов по умолчанию
    algorithmColors["A*"] = QColor(220, 20, 60);
    algorithmColors["BFS (Поиск в Ширину)"] = QColor(34, 139, 34);
    algorithmColors["Дейкстра"] = QColor(25, 25, 112);
    algorithmColors[QString::fromUtf8("Пользователь")] = QColor(255, 235, 59);

    // Настройка таблицы статистики
    QHeaderView* header = ui->table_stats->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);

    ui->table_stats->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->table_stats->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_stats->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_stats->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->table_stats->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->table_stats->resizeColumnsToContents();

    // Элементы управления визуализацией скрыты до включения чекбокса
    ui->spin_vizsp->setVisible(false);
    ui->label_vizsp->setVisible(false);
    ui->lab_tmln->setVisible(false);
    ui->but_ctrlviz->setVisible(false);
    ui->hsl_viz->setVisible(false);
    ui->hsl_viz->setEnabled(false);

    setSolvingControlsVisible(false);
    ui->check_par->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* Показывает/скрывает кнопки движения и метку статистики при прохождении */
void MainWindow::setSolvingControlsVisible(bool visible)
{
    ui->but_up->setVisible(visible);
    ui->but_left->setVisible(visible);
    ui->but_right->setVisible(visible);
    ui->but_down->setVisible(visible);
    ui->lab_self->setVisible(visible);
}

void MainWindow::on_action_triggered()
{
    if (!window) window = new RefWindow(this);
    if (!window->isVisible()) window->show();
    else window->activateWindow();
}

void MainWindow::on_push_alginf_clicked()
{
    if (!infowindow) infowindow = new InfoWindow(this);
    if (!infowindow->isVisible()) infowindow->show();
    else infowindow->activateWindow();
}

void MainWindow::on_spin_height_valueChanged(int arg1)
{
    ui->spin_y2->setMaximum(arg1 - 1);
    ui->spin_y2->setValue(arg1 - 1);
}

void MainWindow::on_spin_width_valueChanged(int arg1)
{
    ui->spin_x2->setMaximum(arg1 - 1);
    ui->spin_x2->setValue(arg1 - 1);
}

/* Пересчитывает допустимые диапазоны для spin_x1/y1/x2/y2 */
void MainWindow::updateCoordinateRanges()
{
    int maxW = ui->spin_width->value() - 1;
    int maxH = ui->spin_height->value() - 1;

    if (maxW < 0) maxW = 0;
    if (maxH < 0) maxH = 0;

    ui->spin_x1->setRange(0, maxW);
    ui->spin_x2->setRange(0, maxW);
    ui->spin_y1->setRange(0, maxH);
    ui->spin_y2->setRange(0, maxH);
}

/**
 * @brief Применяет координаты старта и финиша к лабиринту.
 *
 * Если координаты некорректны (за пределами или совпадают),
 * показывает QMessageBox с описанием проблемы и сбрасывает
 * к значениям по умолчанию.
 */
void MainWindow::applyStartEndCoordinates()
{
    int x1 = ui->spin_x1->value();
    int y1 = ui->spin_y1->value();
    int x2 = ui->spin_x2->value();
    int y2 = ui->spin_y2->value();

    int width = currentMaze.getWidth();
    int height = currentMaze.getHeight();

    Point start{ x1, y1 };
    Point end{ x2, y2 };

    QStringList errors;

    bool startOutOfBounds = (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height);
    bool endOutOfBounds = (x2 < 0 || x2 >= width || y2 < 0 || y2 >= height);

    if (startOutOfBounds) {
        errors << QString::fromUtf8("Координаты начала (%1, %2) выходят за пределы лабиринта (%3×%4).")
            .arg(x1).arg(y1).arg(width).arg(height);
    }
    if (endOutOfBounds) {
        errors << QString::fromUtf8("Координаты конца (%1, %2) выходят за пределы лабиринта (%3×%4).")
            .arg(x2).arg(y2).arg(width).arg(height);
    }

    if (!startOutOfBounds && !endOutOfBounds && start == end) {
        errors << QString::fromUtf8("Координаты начала и конца совпадают (%1, %2).")
            .arg(x1).arg(y1);
    }

    if (!errors.isEmpty()) {
        errors << QString::fromUtf8("Будут использованы координаты по умолчанию: начало (0, 0), конец (%1, %2).")
            .arg(width - 1).arg(height - 1);

        QMessageBox::warning(
            this,
            QString::fromUtf8("Некорректные координаты"),
            errors.join("\n"),
            QMessageBox::Ok
        );

        currentMaze.setStartAndEnd({ 0, 0 }, { width - 1, height - 1 });

        // Синхронизируем UI с новыми значениями
        ui->spin_x1->setValue(0);
        ui->spin_y1->setValue(0);
        ui->spin_x2->setValue(width - 1);
        ui->spin_y2->setValue(height - 1);
        return;
    }

    currentMaze.setStartAndEnd(start, end);
}

/**
 * @brief Запуск или остановка генерации.
 *
 * Работает как переключатель: первое нажатие — старт, второе — остановка.
 * Текст кнопки меняется соответственно.
 */
void MainWindow::on_push_gen_clicked()
{
    if (isGenerating) {
        mazeWidget->stopAnimation();
        statusBar()->showMessage(QString::fromUtf8("Генерация остановлена"), 3000);
        ui->push_gen->setText(QString::fromUtf8("Начать генерацию"));
        isGenerating = false;
        ui->push_solve->setEnabled(false);
        ui->push_self->setEnabled(false);
        return;
    }

    int width = ui->spin_width->value();
    int height = ui->spin_height->value();
    double density = ui->spin_dens->value();
    QString algName = ui->comb_alggen->currentText();

    currentMaze = Maze(width, height);
    applyStartEndCoordinates();

    ui->push_gen->setText(QString::fromUtf8("Остановить генерацию"));
    ui->push_solve->setEnabled(false);
    ui->push_self->setEnabled(false);

    generationTimer.start();
    statusBar()->showMessage(QString::fromUtf8("Генерация начата"));

    std::vector<GenerationStep> steps;
    MazeGeneratorWorker::generate(currentMaze, algName, density, steps);

    // Для анимации нужна "чистая" копия — генератор уже изменил currentMaze
    Maze emptyMaze = currentMaze.createEmptyCopy();

    mazeWidget->clearSolutionPaths();
    mazeWidget->clearUserPath();
    mazeWidget->setMaze(emptyMaze);
    mazeWidget->setGenerationSteps(steps);
    mazeWidget->setPathVisible(false);

    isGenerating = true;

    if (ui->check_viz->isChecked()) {
        mazeWidget->setGenerationDelay(ui->spin_vizsp->value());
    }

    lastVizType = LastVisualization::Generation;
    mazeWidget->startGenerationAnimation();
}

void MainWindow::onGenerationFinished()
{
    double genTime = generationTimer.elapsed();
    statusBar()->showMessage(QString::fromUtf8("Генерация закончена"), 10000);

    if (ui->check_viz->isChecked()) {
        ui->but_ctrlviz->setText(QString::fromUtf8("Пауза"));
        ui->hsl_viz->setEnabled(true);
        ui->hsl_viz->setValue(ui->hsl_viz->maximum());
    }

    ui->push_gen->setText(QString::fromUtf8("Начать генерацию"));
    ui->push_solve->setEnabled(true);
    ui->push_self->setEnabled(true);
    isGenerating = false;

    // Записываем результат в статистику
    QString algName = ui->comb_alggen->currentText();
    StatsEntry entry;
    entry.algorithmName = algName;
    entry.type = StatsEntry::Type::Generator;
    entry.timeMs = genTime;
    entry.pathLength = 0;
    entry.visitedCells = 0;
    entry.color = QColor(128, 128, 128);
    entry.mazeWidth = currentMaze.getWidth();
    entry.mazeHeight = currentMaze.getHeight();
    addStatsEntry(entry);
}

/* Запуск или остановка пользовательского прохождения */
void MainWindow::on_push_self_clicked()
{
    if (isSolving) {
        mazeWidget->stopSolving();
        setSolvingControlsVisible(false);
        ui->push_self->setText(QString::fromUtf8("Начать самостоятельное прохождение"));
        statusBar()->showMessage(QString::fromUtf8("Прохождение остановлено"), 3000);
        isSolving = false;
        ui->push_solve->setEnabled(true);
        ui->push_gen->setEnabled(true);
        return;
    }

    // Очищаем следы предыдущего решения алгоритмом, чтобы не мешали
    mazeWidget->clearSolutionPaths();
    mazeWidget->startSolving();
    setSolvingControlsVisible(true);

    ui->push_self->setText(QString::fromUtf8("Остановить прохождение"));
    statusBar()->showMessage(
        QString::fromUtf8("Прохождение начато. Используйте WASD/стрелки или кнопки"), 3000);
    ui->lab_self->setText(
        QString::fromUtf8("Время: 0.00 сек | Шаги: 0 | Длина пути: 1"));

    isSolving = true;
    ui->push_solve->setEnabled(false);
    ui->push_gen->setEnabled(false);

    lastVizType = LastVisualization::None;
}

void MainWindow::onSolvingFinished(int steps, double timeSeconds, int pathLength)
{
    setSolvingControlsVisible(false);
    ui->push_self->setText(QString::fromUtf8("Начать самостоятельное прохождение"));
    statusBar()->showMessage(
        QString::fromUtf8("Лабиринт пройден! Время: %1 сек | Шаги: %2 | Длина пути: %3")
        .arg(timeSeconds, 0, 'f', 2)
        .arg(steps)
        .arg(pathLength));

    isSolving = false;
    ui->push_solve->setEnabled(true);
    ui->push_gen->setEnabled(true);

    // Записываем статистику пользователя
    StatsEntry entry;
    entry.algorithmName = QString::fromUtf8("Пользователь");
    entry.type = StatsEntry::Type::User;
    entry.timeMs = timeSeconds * 1000.0;
    entry.pathLength = pathLength;
    entry.visitedCells = steps;
    entry.color = QColor(255, 235, 59);
    entry.mazeWidth = currentMaze.getWidth();
    entry.mazeHeight = currentMaze.getHeight();
    addStatsEntry(entry);
}

void MainWindow::onSolvingStatsUpdated(int steps, double timeSeconds, int pathLength)
{
    ui->lab_self->setText(
        QString::fromUtf8("Время: %1 сек | Шаги: %2 | Длина пути: %3")
        .arg(timeSeconds, 0, 'f', 2)
        .arg(steps)
        .arg(pathLength));
}

/**
 * @brief Показывает/скрывает чекбокс параллелизации.
 *
 * Имеет смысл только при выборе 2+ алгоритмов решения.
 * При выборе одного — галочка снимается автоматически.
 */
void MainWindow::onAlgorithmSelectionChanged()
{
    int checkedCount = 0;
    if (ui->check_ast->isChecked()) checkedCount++;
    if (ui->check_bfs->isChecked()) checkedCount++;
    if (ui->check_dej->isChecked()) checkedCount++;

    ui->check_par->setVisible(checkedCount > 1);

    if (checkedCount <= 1) {
        ui->check_par->setChecked(false);
    }
}

/**
 * @brief Запуск или остановка решения лабиринта.
 *
 * При запуске: собирает пути от выбранных алгоритмов,
 * запускает параллельную или последовательную анимацию.
 * При остановке: прерывает анимацию и сбрасывает флаги.
 */
void MainWindow::on_push_solve_clicked()
{
    if (isMazeSolving || isWaitingForNextAlgorithm) {
        mazeWidget->stopSolvingAnimation();
        isWaitingForNextAlgorithm = false;
        statusBar()->showMessage(QString::fromUtf8("Решение остановлено"), 3000);
        ui->push_solve->setText(QString::fromUtf8("Начать решение лабиринта"));
        isMazeSolving = false;
        ui->but_ctrlviz->setText(QString::fromUtf8("Пауза"));
        ui->but_ctrlviz->setEnabled(false);
        ui->push_gen->setEnabled(true);
        ui->push_self->setEnabled(true);
        return;
    }

    statusBar()->showMessage(QString::fromUtf8("Решение начато"));
    ui->push_solve->setText(QString::fromUtf8("Остановить решение лабиринта"));

    std::vector<SolutionPath> paths = solveWithAlgorithms();

    if (paths.empty()) {
        statusBar()->showMessage(QString::fromUtf8("Не выбран ни один алгоритм или путь не найден"), 10000);
        ui->push_solve->setText(QString::fromUtf8("Начать решение лабиринта"));
        return;
    }

    mazeWidget->clearUserPath();

    pendingPaths = paths;
    currentSequentialIndex = 0;
    isParallelSolving = ui->check_par->isChecked();
    lastVizType = LastVisualization::Solving;

    if (isParallelSolving) {
        mazeWidget->setSolutionPaths(paths);
        isMazeSolving = true;
        mazeWidget->startSolvingAnimation();
    }
    else {
        std::vector<SolutionPath> firstPath = { paths[0] };
        mazeWidget->setSolutionPaths(firstPath);
        isMazeSolving = true;
        ui->push_gen->setEnabled(false);
        ui->push_self->setEnabled(false);
        mazeWidget->startSolvingAnimation();
    }
}

/**
 * @brief Обрабатывает завершение одного шага решения.
 *
 * В последовательном режиме: записывает статистику завершённого алгоритма,
 * делает паузу 1 секунду и запускает следующий.
 * В параллельном режиме: ничего не делает (все уже завершились).
 */
void MainWindow::onMazeSolvingFinished()
{
    if (!isParallelSolving && currentSequentialIndex + 1 < pendingPaths.size()) {
        const SolutionPath& finishedPath = pendingPaths[currentSequentialIndex];
        StatsEntry entry;
        entry.algorithmName = finishedPath.algorithmName;
        entry.type = StatsEntry::Type::Solver;
        entry.timeMs = finishedPath.solveTimeMs;
        entry.pathLength = static_cast<int>(finishedPath.path.size());
        entry.visitedCells = static_cast<int>(finishedPath.visitedOrder.size());
        entry.color = finishedPath.pathColor;
        entry.mazeWidth = currentMaze.getWidth();
        entry.mazeHeight = currentMaze.getHeight();
        addStatsEntry(entry);

        QString finishedAlgName = pendingPaths[currentSequentialIndex].algorithmName;

        isWaitingForNextAlgorithm = true;
        statusBar()->showMessage(
            QString::fromUtf8("Алгоритм %1 завершён. Пауза перед следующим...")
            .arg(finishedAlgName),
            1000);

        // Неблокирующий таймер — UI остаётся отзывчивым
        QTimer::singleShot(1000, this, [this]() {
            if (!isWaitingForNextAlgorithm) {
                return;
            }
            isWaitingForNextAlgorithm = false;

            currentSequentialIndex++;
            std::vector<SolutionPath> nextPath = { pendingPaths[currentSequentialIndex] };
            mazeWidget->setSolutionPaths(nextPath);

            isMazeSolving = true;
            mazeWidget->startSolvingAnimation();

            QString nextAlgName = pendingPaths[currentSequentialIndex].algorithmName;
            statusBar()->showMessage(
                QString::fromUtf8("Запуск алгоритма %1...").arg(nextAlgName),
                5000);
            });
        return;
    }

    // Финальная запись статистики
    if (isParallelSolving) {
        for (const auto& path : pendingPaths) {
            StatsEntry entry;
            entry.algorithmName = path.algorithmName;
            entry.type = StatsEntry::Type::Solver;
            entry.timeMs = path.solveTimeMs;
            entry.pathLength = static_cast<int>(path.path.size());
            entry.visitedCells = static_cast<int>(path.visitedOrder.size());
            entry.color = path.pathColor;
            entry.mazeWidth = currentMaze.getWidth();
            entry.mazeHeight = currentMaze.getHeight();
            addStatsEntry(entry);
        }
    }
    else {
        const SolutionPath& lastPath = pendingPaths[currentSequentialIndex];
        StatsEntry entry;
        entry.algorithmName = lastPath.algorithmName;
        entry.type = StatsEntry::Type::Solver;
        entry.timeMs = lastPath.solveTimeMs;
        entry.pathLength = static_cast<int>(lastPath.path.size());
        entry.visitedCells = static_cast<int>(lastPath.visitedOrder.size());
        entry.color = lastPath.pathColor;
        entry.mazeWidth = currentMaze.getWidth();
        entry.mazeHeight = currentMaze.getHeight();
        addStatsEntry(entry);
    }

    statusBar()->showMessage(QString::fromUtf8("Решение закончено"), 3000);
    ui->push_solve->setText(QString::fromUtf8("Начать решение лабиринта"));
    isMazeSolving = false;
    ui->push_self->setEnabled(true);
    ui->push_gen->setEnabled(true);
    ui->but_ctrlviz->setText(QString::fromUtf8("Пауза"));
    ui->hsl_viz->setEnabled(ui->check_viz->isChecked());
    ui->hsl_viz->setValue(ui->hsl_viz->maximum());
}

/**
 * @brief Запускает выбранные алгоритмы решения и собирает их результаты.
 *
 * Если отмечены чекбоксы — использует их. Иначе берёт алгоритм из ComboBox.
 * Для каждого алгоритма замеряет время CPU через QElapsedTimer.
 * Цвета берутся из algorithmColors (могут быть изменены пользователем в таблице).
 */
std::vector<SolutionPath> MainWindow::solveWithAlgorithms()
{
    std::vector<SolutionPath> paths;

    bool useAstar = (ui->check_ast->isChecked() && ui->check_sevalg->isChecked());
    bool useBfs = (ui->check_bfs->isChecked() && ui->check_sevalg->isChecked());
    bool useDijkstra = (ui->check_dej->isChecked() && ui->check_sevalg->isChecked());

    if (!useAstar && !useBfs && !useDijkstra) {
        QString algName = ui->comb_algsol->currentText();
        if (algName == "A*") useAstar = true;
        else if (algName == "BFS (Поиск в Ширину)") useBfs = true;
        else if (algName == "Дейкстра") useDijkstra = true;
    }

    AStarSolver astarSolver;
    BFSSolver bfsSolver;
    DijkstraSolver dijkstraSolver;

    // Яркий цвет для "фронта" — клеток, которые алгоритм сейчас исследует
    auto makeFrontierColor = [](const QColor& baseColor) -> QColor {
        QColor hsl = baseColor.toHsl();
        int newLightness = std::min(255, static_cast<int>(hsl.lightness() * 1.4));
        int newSaturation = std::min(255, static_cast<int>(hsl.saturation() * 1.3));
        return QColor::fromHsl(hsl.hue(), newSaturation, newLightness, 230);
        };

    // Полупрозрачный цвет для уже посещённых клеток
    auto makeVisitedColor = [](const QColor& baseColor) -> QColor {
        return QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 90);
        };

    if (useAstar) {
        QElapsedTimer t;
        t.start();
        SolvingResult result = astarSolver.solve(currentMaze);
        double elapsed = t.nsecsElapsed() / 1000000.0;
        if (result.found) {
            QColor pathColor = algorithmColors["A*"];
            paths.push_back({
                result.path, result.visitedOrder,
                pathColor,
                makeVisitedColor(pathColor),
                makeFrontierColor(pathColor),
                "A*",
                elapsed
                });
        }
    }

    if (useBfs) {
        QElapsedTimer t;
        t.start();
        SolvingResult result = bfsSolver.solve(currentMaze);
        double elapsed = t.nsecsElapsed() / 1000000.0;
        if (result.found) {
            QColor pathColor = algorithmColors["BFS (Поиск в Ширину)"];
            paths.push_back({
                result.path, result.visitedOrder,
                pathColor,
                makeVisitedColor(pathColor),
                makeFrontierColor(pathColor),
                "BFS (Поиск в Ширину)",
                elapsed
                });
        }
    }

    if (useDijkstra) {
        QElapsedTimer t;
        t.start();
        SolvingResult result = dijkstraSolver.solve(currentMaze);
        double elapsed = t.nsecsElapsed() / 1000000.0;
        if (result.found) {
            QColor pathColor = algorithmColors["Дейкстра"];
            paths.push_back({
                result.path, result.visitedOrder,
                pathColor,
                makeVisitedColor(pathColor),
                makeFrontierColor(pathColor),
                "Дейкстра",
                elapsed
                });
        }
    }

    return paths;
}

void MainWindow::addStatsEntry(const StatsEntry& entry)
{
    allStats.push_back(entry);
    refreshStatsTable();
}

/* Перерисовывает таблицу с учётом текущего фильтра */
void MainWindow::refreshStatsTable()
{
    filteredStats.clear();
    for (const auto& entry : allStats) {
        bool include = false;
        switch (currentFilter) {
        case 0: include = true; break;
        case 1: include = entry.isGenerator(); break;
        case 2: include = entry.isSolver() || entry.isUser(); break;
        case 3: include = entry.isUser(); break;
        }
        if (include) {
            filteredStats.push_back(entry);
        }
    }

    ui->table_stats->setRowCount(static_cast<int>(filteredStats.size()));

    for (int row = 0; row < static_cast<int>(filteredStats.size()); ++row) {
        const StatsEntry& entry = filteredStats[row];

        QTableWidgetItem* nameItem = new QTableWidgetItem(entry.algorithmName);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->table_stats->setItem(row, 0, nameItem);

        QTableWidgetItem* sizeItem = new QTableWidgetItem(entry.mazeSizeString());
        sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
        sizeItem->setTextAlignment(Qt::AlignCenter);
        ui->table_stats->setItem(row, 1, sizeItem);

        QTableWidgetItem* timeItem = new QTableWidgetItem(formatTime(entry.timeMs));
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        timeItem->setTextAlignment(Qt::AlignCenter);
        ui->table_stats->setItem(row, 2, timeItem);

        QString pathText = entry.isGenerator() ? "-" : QString::number(entry.pathLength);
        QTableWidgetItem* pathItem = new QTableWidgetItem(pathText);
        pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
        pathItem->setTextAlignment(Qt::AlignCenter);
        ui->table_stats->setItem(row, 3, pathItem);

        QString visitedText = entry.isGenerator() ? "-" : QString::number(entry.visitedCells);
        QTableWidgetItem* visitedItem = new QTableWidgetItem(visitedText);
        visitedItem->setFlags(visitedItem->flags() & ~Qt::ItemIsEditable);
        visitedItem->setTextAlignment(Qt::AlignCenter);
        ui->table_stats->setItem(row, 4, visitedItem);

        QTableWidgetItem* colorItem = new QTableWidgetItem();
        colorItem->setBackground(entry.color);
        if (entry.isSolver() || entry.isUser()) {
            colorItem->setFlags(colorItem->flags() | Qt::ItemIsEnabled);
            colorItem->setToolTip(QString::fromUtf8("Нажмите для изменения цвета"));
        }
        else {
            colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEnabled);
        }
        ui->table_stats->setItem(row, 5, colorItem);
    }

    ui->table_stats->resizeColumnsToContents();

    // Не даём колонкам сжаться до нечитаемого размера
    for (int col = 0; col < ui->table_stats->columnCount(); ++col) {
        if (ui->table_stats->columnWidth(col) < 80) {
            ui->table_stats->setColumnWidth(col, 80);
        }
    }
}

/* Форматирует время в зависимости от величины: мкс, мс, сек, мин */
QString MainWindow::formatTime(double timeMs) const
{
    if (timeMs < 1) {
        return QString::number(timeMs * 1000.0, 'f', 1) + QString::fromUtf8(" мкс");
    }
    else if (timeMs < 1000) {
        return QString::number(timeMs, 'f', 3) + QString::fromUtf8(" мс");
    }
    else if (timeMs < 60000) {
        return QString::number(timeMs / 1000.0, 'f', 3) + QString::fromUtf8(" сек");
    }
    else {
        int minutes = static_cast<int>(timeMs / 60000);
        double seconds = (timeMs - minutes * 60000) / 1000.0;
        return QString("%1 мин %2 сек").arg(minutes).arg(seconds, 0, 'f', 2);
    }
}

/**
 * @brief Клик по ячейке таблицы.
 *
 * Интересует только колонка с цветом (индекс 5).
 * Открывает QColorDialog и применяет новый цвет ко всем
 * записям этого алгоритма в таблице и к текущей визуализации.
 */
void MainWindow::on_table_stats_cellClicked(int row, int column)
{
    if (column != 5) return;
    if (row < 0 || row >= static_cast<int>(filteredStats.size())) return;

    StatsEntry& entry = filteredStats[row];
    if (entry.isGenerator()) return;

    QColor newColor = QColorDialog::getColor(entry.color, this,
        QString::fromUtf8("Выберите цвет"));
    if (!newColor.isValid()) return;

    QString algName = entry.algorithmName;
    StatsEntry::Type entryType = entry.type;

    // Сохраняем в глобальное хранилище — пригодится при следующем запуске
    algorithmColors[algName] = newColor;

    for (auto& origEntry : allStats) {
        if (origEntry.algorithmName == algName && origEntry.type == entryType) {
            origEntry.color = newColor;
        }
    }

    for (auto& filteredEntry : filteredStats) {
        if (filteredEntry.algorithmName == algName && filteredEntry.type == entryType) {
            filteredEntry.color = newColor;
        }
    }

    mazeWidget->updateAlgorithmColor(algName, newColor);
    refreshStatsTable();
}

void MainWindow::on_but_clear_stats_clicked()
{
    allStats.clear();
    filteredStats.clear();
    ui->table_stats->setRowCount(0);
}

void MainWindow::on_combo_filter_currentIndexChanged(int index)
{
    currentFilter = index;
    refreshStatsTable();
}

/* Показывает/скрывает элементы управления визуализацией */
void MainWindow::on_check_viz_toggled(bool checked)
{
    ui->spin_vizsp->setVisible(checked);
    ui->label_vizsp->setVisible(checked);
    ui->lab_tmln->setVisible(checked);
    ui->but_ctrlviz->setVisible(checked);
    ui->hsl_viz->setVisible(checked);

    if (checked) {
        bool solvingActive = mazeWidget->isMazeSolvingActive();
        bool generatingActive = mazeWidget->isGeneratingActive();
        bool anyActive = solvingActive || generatingActive;

        mazeWidget->setAnimationDelay(ui->spin_vizsp->value());
        mazeWidget->setGenerationDelay(ui->spin_vizsp->value());

        // Синхронизируем слайдер с текущим состоянием
        if (generatingActive) {
            int total = mazeWidget->getTotalGenerationSteps();
            int current = mazeWidget->getCurrentGenerationStep();
            ui->hsl_viz->setRange(0, total);
            ui->hsl_viz->setValue(current);
        }
        else if (solvingActive) {
            int total = mazeWidget->getTotalSteps();
            int current = mazeWidget->getCurrentStep();
            ui->hsl_viz->setRange(0, total);
            ui->hsl_viz->setValue(current);
        }

        ui->hsl_viz->setEnabled(!anyActive);
        updateVizControlsState();
    }
    else {
        // Возвращаем стандартную скорость
        mazeWidget->setAnimationDelay(0);
        mazeWidget->setGenerationDelay(0);

        if (mazeWidget->isPaused()) {
            if (mazeWidget->isMazeSolvingActive()) {
                mazeWidget->resumeVisualization();
            }
            else if (mazeWidget->isGeneratingActive()) {
                mazeWidget->resumeGeneration();
            }
        }

        ui->but_ctrlviz->setText(QString::fromUtf8("Пауза"));
        ui->hsl_viz->setEnabled(false);
    }
}

/* Переключает паузу/возобновление визуализации */
void MainWindow::on_but_ctrlviz_clicked()
{
    if (!ui->check_viz->isChecked()) return;

    bool generatingActive = mazeWidget->isGeneratingActive();
    bool solvingActive = mazeWidget->isMazeSolvingActive();

    if (mazeWidget->isPaused()) {
        int targetStep = ui->hsl_viz->value();
        if (generatingActive) {
            mazeWidget->seekGenerationTo(targetStep);
            mazeWidget->resumeGeneration();
        }
        else if (solvingActive) {
            mazeWidget->seekTo(targetStep);
            mazeWidget->resumeVisualization();
        }
        ui->but_ctrlviz->setText(QString::fromUtf8("Пауза"));
    }
    else {
        if (generatingActive) {
            mazeWidget->pauseGeneration();
        }
        else if (solvingActive) {
            mazeWidget->pauseVisualization();
        }
        ui->but_ctrlviz->setText(QString::fromUtf8("Возобновить"));
    }

    updateVizControlsState();
}

void MainWindow::on_spin_vizsp_valueChanged(int value)
{
    if (!ui->check_viz->isChecked()) return;
    mazeWidget->setAnimationDelay(value);
    mazeWidget->setGenerationDelay(value);
}

/**
 * @brief Обработка перемещения слайдера таймлайна.
 *
 * Если анимация идёт — игнорируем (она сама управляет позицией).
 * Если на паузе или завершена — перематываем к нужному шагу.
 */
void MainWindow::on_hsl_viz_valueChanged(int value)
{
    if (!ui->check_viz->isChecked()) return;

    bool solvingActive = mazeWidget->isMazeSolvingActive();
    bool generatingActive = mazeWidget->isGeneratingActive();
    bool anyActive = solvingActive || generatingActive;

    if (anyActive && !mazeWidget->isPaused()) {
        return;
    }

    if (generatingActive && mazeWidget->isPaused()) {
        mazeWidget->seekGenerationTo(value);
    }
    else if (solvingActive && mazeWidget->isPaused()) {
        mazeWidget->seekTo(value);
    }
    else if (!anyActive) {
        // Визуализация завершена — перемотка для просмотра
        if (lastVizType == LastVisualization::Generation) {
            mazeWidget->seekGenerationTo(value);
        }
        else if (lastVizType == LastVisualization::Solving) {
            mazeWidget->seekTo(value);
        }
    }

    updateVizControlsState();
}

void MainWindow::onVisualizationProgress(int current, int total)
{
    ui->hsl_viz->setRange(0, total);
    if (!ui->hsl_viz->isSliderDown()) {
        ui->hsl_viz->setValue(current);
    }

    if (ui->check_viz->isChecked()) {
        bool anyActive = mazeWidget->isMazeSolvingActive() ||
            mazeWidget->isGeneratingActive();
        ui->hsl_viz->setEnabled(!anyActive);
    }

    updateVizControlsState();
}

void MainWindow::onGenerationVisualizationProgress(int current, int total)
{
    ui->hsl_viz->setRange(0, total);
    if (!ui->hsl_viz->isSliderDown()) {
        ui->hsl_viz->setValue(current);
    }

    if (ui->check_viz->isChecked()) {
        bool anyActive = mazeWidget->isMazeSolvingActive() ||
            mazeWidget->isGeneratingActive();
        ui->hsl_viz->setEnabled(!anyActive);
    }

    updateVizControlsState();
}

/* Обновляет enabled/disabled у элементов управления визуализацией */
void MainWindow::updateVizControlsState()
{
    bool vizEnabled = ui->check_viz->isChecked();
    bool solvingActive = mazeWidget->isMazeSolvingActive();
    bool generatingActive = mazeWidget->isGeneratingActive();
    bool anyVizActive = solvingActive || generatingActive;

    // Слайдер активен только после завершения визуализации
    bool sliderEnabled = vizEnabled && !anyVizActive;
    ui->hsl_viz->setEnabled(sliderEnabled);

    // Кнопка паузы — только если визуализация идёт и не в конце
    bool sliderAtEnd = (ui->hsl_viz->value() >= ui->hsl_viz->maximum()) &&
        (ui->hsl_viz->maximum() > 0);
    bool pauseButtonEnabled = vizEnabled && anyVizActive && !sliderAtEnd;
    ui->but_ctrlviz->setEnabled(pauseButtonEnabled);

    ui->spin_vizsp->setEnabled(vizEnabled);
    ui->label_vizsp->setEnabled(vizEnabled);
    ui->lab_tmln->setEnabled(vizEnabled);
}

void MainWindow::on_but_up_clicked()
{
    mazeWidget->movePlayer(Direction::North);
}

void MainWindow::on_but_left_clicked()
{
    mazeWidget->movePlayer(Direction::West);
}

void MainWindow::on_but_right_clicked()
{
    mazeWidget->movePlayer(Direction::East);
}

void MainWindow::on_but_down_clicked()
{
    mazeWidget->movePlayer(Direction::South);
}

/**
 * @brief Обработка нажатий клавиш при прохождении.
 *
 * Стрелки работают при любой раскладке.
 * WASD — через нативные коды клавиш (не зависят от раскладки).
 * Если прохождение не активно — передаём событие дальше.
 */
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (!isSolving) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    // Стрелки — универсальны
    if (event->key() == Qt::Key_Up) {
        mazeWidget->movePlayer(Direction::North);
        return;
    }
    if (event->key() == Qt::Key_Down) {
        mazeWidget->movePlayer(Direction::South);
        return;
    }
    if (event->key() == Qt::Key_Left) {
        mazeWidget->movePlayer(Direction::West);
        return;
    }
    if (event->key() == Qt::Key_Right) {
        mazeWidget->movePlayer(Direction::East);
        return;
    }

#ifdef Q_OS_WIN
    // Виртуальные коды клавиш Windows — не зависят от раскладки
    quint32 nativeKey = event->nativeVirtualKey();
    const quint32 VK_W = 0x57;
    const quint32 VK_A = 0x41;
    const quint32 VK_S = 0x53;
    const quint32 VK_D = 0x44;

    if (nativeKey == VK_W) { mazeWidget->movePlayer(Direction::North); return; }
    if (nativeKey == VK_S) { mazeWidget->movePlayer(Direction::South); return; }
    if (nativeKey == VK_A) { mazeWidget->movePlayer(Direction::West); return; }
    if (nativeKey == VK_D) { mazeWidget->movePlayer(Direction::East); return; }
#else
    quint32 scanCode = event->nativeScanCode();

#ifdef Q_OS_LINUX
    const quint32 SCAN_W = 25;
    const quint32 SCAN_A = 38;
    const quint32 SCAN_S = 39;
    const quint32 SCAN_D = 40;

    if (scanCode == SCAN_W) { mazeWidget->movePlayer(Direction::North); return; }
    if (scanCode == SCAN_S) { mazeWidget->movePlayer(Direction::South); return; }
    if (scanCode == SCAN_A) { mazeWidget->movePlayer(Direction::West); return; }
    if (scanCode == SCAN_D) { mazeWidget->movePlayer(Direction::East); return; }
#endif

#ifdef Q_OS_MACOS
    const quint32 SCAN_W = 13;
    const quint32 SCAN_A = 0;
    const quint32 SCAN_S = 1;
    const quint32 SCAN_D = 2;

    if (scanCode == SCAN_W) { mazeWidget->movePlayer(Direction::North); return; }
    if (scanCode == SCAN_S) { mazeWidget->movePlayer(Direction::South); return; }
    if (scanCode == SCAN_A) { mazeWidget->movePlayer(Direction::West); return; }
    if (scanCode == SCAN_D) { mazeWidget->movePlayer(Direction::East); return; }
#endif
#endif

    QMainWindow::keyPressEvent(event);
}
