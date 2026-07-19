#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QDialog>
#include <QMovie>
#include <QMap>
#include <QString>

struct AlgorithmInfo {
    QString title;
    QString authorYear;
    QString referenceUrl;
    QString description;
    QString gifPath;
    QString schemaPath;
};

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = nullptr);
    ~InfoWindow();

private slots:
    void onAlgorithmButtonClicked(const QString &algorithmName);

private:
    void setupAlgorithmData();
    void loadAlgorithmInfo(const QString &algorithmName);
    void updateButtonStyles(const QString &activeButtonName);

    Ui::InfoWindow *ui;
    QMap<QString, AlgorithmInfo> algorithmData;
    QMovie *currentMovie;
    QString currentAlgorithm;
    QSize calculateFittedSize(const QSize &origSize, int maxWidth, int maxHeight);
};

#endif
