#ifndef STATS_ENTRY_H
#define STATS_ENTRY_H

#include <QString>
#include <QColor>

struct StatsEntry {
    enum class Type {
        Generator,
        Solver,
        User
    };

    QString algorithmName;
    Type type;
    double timeMs;
    int pathLength;
    int visitedCells;
    QColor color;
    int mazeWidth;
    int mazeHeight;

    bool isGenerator() const { return type == Type::Generator; }
    bool isSolver() const { return type == Type::Solver; }
    bool isUser() const { return type == Type::User; }

    QString mazeSizeString() const {
        return QString("%1 x %2").arg(mazeWidth).arg(mazeHeight);
    }
};

#endif
