#include "mainwindow.h"
#include <QApplication>

/**
 * @brief Точка входа приложения.
 *
 * Создаёт QApplication и главное окно. Вся инициализация
 * виджетов, стилей и ресурсов происходит в конструкторе MainWindow.
 */
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}