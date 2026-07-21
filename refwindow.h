#ifndef REFWINDOW_H
#define REFWINDOW_H

#include <QDialog>

namespace Ui {
    class RefWindow;
}

/* Окно со списком литературы и использованных источников.
   Открывается из главного меню приложения. */
class RefWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RefWindow(QWidget* parent = nullptr);
    ~RefWindow();

private:
    Ui::RefWindow* ui;
};

#endif