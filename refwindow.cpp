#include "refwindow.h"
#include "ui_refwindow.h"

RefWindow::RefWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RefWindow)
{
    ui->setupUi(this);
}

RefWindow::~RefWindow()
{
    delete ui;
}
