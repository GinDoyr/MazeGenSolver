#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <vector>
#include <QTableWidgetItem>
#include "refwindow.h"
#include "infowindow.h"
#include "maze.h"
#include "maze_widget.h"
#include "stats_entry.h"

namespace Ui {
    class MainWindow;
}

/**
 * @brief Главное окно приложения.
 *
 * Координирует работу всех компонентов:
 * - принимает параметры от UI и запускает генерацию/решение/прохождение
 * - хранит состояние (что сейчас запущено, какой тип визуализации последний)
 * - ведёт таблицу статистики
 * - обрабатывает клавиатурный ввод при прохождении
 *
 * Вся логика алгоритмов вынесена в отдельные классы (Generator, ISolver),
 * здесь только управление потоком и обновление UI.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Сигналы от MazeWidget о прогрессе визуализации
    void onVisualizationProgress(int current, int total);
    void onGenerationVisualizationProgress(int current, int total);

    // Изменение выбора алгоритмов решения (чекбоксы)
    void onAlgorithmSelectionChanged();

    // Обновление статистики во время пользовательского прохождения
    void onSolvingStatsUpdated(int steps, double timeSeconds, int pathLength);
    void onSolvingFinished(int steps, double timeSeconds, int pathLength);
    void onGenerationFinished();
    void onMazeSolvingFinished();

    // Открытие вспомогательных окон
    void on_action_triggered();
    void on_push_alginf_clicked();

    // Обработчики UI-элементов (автоматически подключаются Qt Designer)
    void on_spin_height_valueChanged(int arg1);
    void on_spin_width_valueChanged(int arg1);
    void on_push_gen_clicked();
    void on_push_self_clicked();
    void on_but_up_clicked();
    void on_but_left_clicked();
    void on_but_right_clicked();
    void on_but_down_clicked();
    void on_push_solve_clicked();

    // Статистика и управление визуализацией
    void on_table_stats_cellClicked(int row, int column);
    void on_but_clear_stats_clicked();
    void on_combo_filter_currentIndexChanged(int index);
    void on_check_viz_toggled(bool checked);
    void on_but_ctrlviz_clicked();
    void on_spin_vizsp_valueChanged(int value);
    void on_hsl_viz_valueChanged(int value);

private:
    /**
     * @brief Тип последней запущенной визуализации.
     *
     * Нужен, чтобы слайдер таймлайна знал, какую анимацию перематывать
     * после завершения. Иначе непонятно: генерация это была или решение.
     */
    enum class LastVisualization { None, Generation, Solving };
    LastVisualization lastVizType;

    // Вспомогательные методы
    void updateCoordinateRanges();
    void applyStartEndCoordinates();
    void setSolvingControlsVisible(bool visible);
    void updateParallelCheckBoxVisibility();

    /* Запускает выбранные алгоритмы решения и собирает их результаты */
    std::vector<SolutionPath> solveWithAlgorithms();

    // Работа со статистикой
    void addStatsEntry(const StatsEntry& entry);
    void refreshStatsTable();

    // Обновление enabled/disabled у элементов управления визуализацией
    void updateVizControlsState();

    /* Форматирует миллисекунды в читаемую строку (мкс/мс/сек/мин) */
    QString formatTime(double timeMs) const;

    Ui::MainWindow* ui;
    RefWindow* window;
    InfoWindow* infowindow;
    MazeWidget* mazeWidget;
    Maze currentMaze;

    // Флаги текущего состояния — что именно сейчас запущено
    bool isGenerating;
    bool isSolving;
    bool isMazeSolving;

    // Управление последовательным/параллельным режимом решения
    std::vector<SolutionPath> pendingPaths;
    size_t currentSequentialIndex;
    bool isParallelSolving;
    bool isWaitingForNextAlgorithm;

    // Статистика
    std::vector<StatsEntry> allStats;
    std::vector<StatsEntry> filteredStats;
    int currentFilter;
    double generationStartTime;
    QElapsedTimer generationTimer;

    /**
     * @brief Цвета алгоритмов, сохраняемые между запусками.
     *
     * Когда пользователь меняет цвет в таблице, он пишется сюда.
     * При следующем запуске алгоритма цвет берётся из этой карты,
     * а не из захардкоженных значений.
     */
    QMap<QString, QColor> algorithmColors;
};

#endif