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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onVisualizationProgress(int current, int total);
    void onGenerationVisualizationProgress(int current, int total);
    void onAlgorithmSelectionChanged();
    void onSolvingStatsUpdated(int steps, double timeSeconds, int pathLength);
    void onSolvingFinished(int steps, double timeSeconds, int pathLength);
    void onGenerationFinished();
    void onMazeSolvingFinished();

    void on_action_triggered();
    void on_push_alginf_clicked();
    void on_spin_height_valueChanged(int arg1);
    void on_spin_width_valueChanged(int arg1);
    void on_push_gen_clicked();
    void on_push_self_clicked();
    void on_but_up_clicked();
    void on_but_left_clicked();
    void on_but_right_clicked();
    void on_but_down_clicked();
    void on_push_solve_clicked();

    // Слоты для статистики и управления визуализацией
    void on_table_stats_cellClicked(int row, int column);
    void on_but_clear_stats_clicked();
    void on_combo_filter_currentIndexChanged(int index);
    void on_check_viz_toggled(bool checked);
    void on_but_ctrlviz_clicked();
    void on_spin_vizsp_valueChanged(int value);
    void on_hsl_viz_valueChanged(int value);

private:

    enum class LastVisualization { None, Generation, Solving };
    LastVisualization lastVizType;

    void updateCoordinateRanges();
    void applyStartEndCoordinates();
    void setSolvingControlsVisible(bool visible);
    void updateParallelCheckBoxVisibility();
    std::vector<SolutionPath> solveWithAlgorithms();
    void addStatsEntry(const StatsEntry &entry);
    void refreshStatsTable();
    void updateVizControlsState();
    QString formatTime(double timeMs) const;

    Ui::MainWindow *ui;
    RefWindow *window;
    InfoWindow *infowindow;
    MazeWidget *mazeWidget;
    Maze currentMaze;
    bool isGenerating;
    bool isSolving;
    bool isMazeSolving;

    // поля для управления последовательным/параллельным режимом
    std::vector<SolutionPath> pendingPaths;   // Все пути для последовательного режима
    size_t currentSequentialIndex;            // Индекс текущего алгоритма
    bool isParallelSolving;
    bool isWaitingForNextAlgorithm;

    std::vector<StatsEntry> allStats;          // Все записи статистики
    std::vector<StatsEntry> filteredStats;     // Отфильтрованные записи
    int currentFilter;                          // 0=все, 1=генераторы, 2=решатели, 3=пользователь
    double generationStartTime;                 // Для замера времени генерации
    QElapsedTimer generationTimer;              // Таймер генерации

    // Хранилище цветов алгоритмов (сохраняется между запусками)
    QMap<QString, QColor> algorithmColors;
};
#endif
