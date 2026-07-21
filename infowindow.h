#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QDialog>
#include <QMovie>
#include <QMap>
#include <QString>

/**
 * @brief Данные об одном алгоритме для отображения в окне справки.
 *
 * Все поля — строки или пути к ресурсам Qt. Заполняются один раз
 * в setupAlgorithmData() и потом только читаются.
 */
struct AlgorithmInfo {
    QString title;          ///< Название
    QString authorYear;     ///< Автор и год (читаемый формат для UI)
    QString referenceUrl;   ///< Ссылка на источник
    QString description;    ///< Описание принципа работы
    QString application;    ///< Где применяется
    QString gifPath;        ///< Путь к GIF в ресурсах Qt (:/resources/...)
    QString schemaPath;     ///< Путь к блок-схеме в ресурсах Qt
};

namespace Ui {
    class InfoWindow;
}

/**
 * @brief Окно со справкой по алгоритмам.
 *
 * Слева — кнопки выбора, справа — описание, GIF и блок-схема.
 * Переключение между алгоритмами через QButtonGroup.
 */
class InfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget* parent = nullptr);
    ~InfoWindow();

private slots:
    void onAlgorithmButtonClicked(const QString& algorithmName);

private:
    void setupAlgorithmData();
    void loadAlgorithmInfo(const QString& algorithmName);
    void updateButtonStyles(const QString& activeButtonName);

    Ui::InfoWindow* ui;
    QMap<QString, AlgorithmInfo> algorithmData;
    QMovie* currentMovie;
    QString currentAlgorithm;

    /**
     * @brief Подбирает размер изображения под ограничения.
     *
     * Сначала пытается вписать по высоте. Если ширина вылезает —
     * масштабирует по ширине. Пропорции сохраняются.
     */
    QSize calculateFittedSize(const QSize& origSize, int maxWidth, int maxHeight);
};

#endif