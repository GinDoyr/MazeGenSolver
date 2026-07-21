#ifndef STATS_ENTRY_H
#define STATS_ENTRY_H

#include <QString>
#include <QColor>

/* Одна запись в таблице статистики.
   Заполняется при завершении генерации, решения или пользовательского прохождения.
   Для генераторов pathLength и visitedCells не используются — в таблице показывается "-". */
struct StatsEntry {
    enum class Type {
        Generator,  /* Алгоритм генерации лабиринта */
        Solver,     /* Алгоритм решения лабиринта */
        User        /* Пользовательское прохождение */
    };

    QString algorithmName;   /* Имя алгоритма или "Пользователь" */
    Type type;
    double timeMs;           /* Время работы CPU в миллисекундах */
    int pathLength;          /* Длина найденного пути (0 для генераторов) */
    int visitedCells;        /* Количество посещённых клеток (0 для генераторов) */
    QColor color;            /* Цвет алгоритма в визуализации */
    int mazeWidth;           /* Размер лабиринта по X */
    int mazeHeight;          /* Размер лабиринта по Y */

    /* Вспомогательные методы для фильтрации таблицы */
    bool isGenerator() const { return type == Type::Generator; }
    bool isSolver() const { return type == Type::Solver; }
    bool isUser() const { return type == Type::User; }

    /* Форматирует размер лабиринта для отображения в таблице */
    QString mazeSizeString() const {
        return QString("%1 x %2").arg(mazeWidth).arg(mazeHeight);
    }
};

#endif