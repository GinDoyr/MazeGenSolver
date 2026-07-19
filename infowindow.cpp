#include "infowindow.h"
#include "ui_infowindow.h"
#include <QPixmap>
#include <QButtonGroup>

InfoWindow::InfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow)
    , currentMovie(nullptr)
{
    ui->setupUi(this);

    setupAlgorithmData();

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->but_dfs, 0);
    buttonGroup->addButton(ui->but_kr, 1);
    buttonGroup->addButton(ui->but_pr, 2);
    buttonGroup->addButton(ui->but_ast, 3);
    buttonGroup->addButton(ui->but_dej, 4);
    buttonGroup->addButton(ui->but_bfs, 5);

    connect(buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton *button) {
                onAlgorithmButtonClicked(button->text());
            });

    onAlgorithmButtonClicked(ui->but_dfs->text());
}

InfoWindow::~InfoWindow()
{
    delete ui;
    delete currentMovie;
}

void InfoWindow::setupAlgorithmData()
{
    // DFS (Поиск в глубину)
    algorithmData["DFS (Поиск в Глубину)"] = {
        "DFS (Depth-First Search) - Поиск в глубину",
        "Шарль Пьер Тремо, 1859 г.",
        "https://ru.wikipedia.org/wiki/%D0%9F%D0%BE%D0%B8%D1%81%D0%BA_%D0%B2_%D0%B3%D0%BB%D1%83%D0%B1%D0%B8%D0%BD%D1%83",
        "Алгоритм начинает с произвольной клетки и идёт вглубь лабиринта, пока не упрётся в тупик. Затем отступает назад (backtracking) и пробует другое направление. Использует стек для отслеживания пути. Создаёт лабиринты с длинными извилистыми коридорами.",
        ":/resources/gifs/dfs.gif",
        ":/resources/img/dfs.png"
    };

    // Крускал
    algorithmData["Краскал"] = {
        "Алгоритм Краскала",
        "Джозеф Крускал, 1956 г.",
        "https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%9A%D1%80%D0%B0%D1%81%D0%BA%D0%B0%D0%BB%D0%B0",
        "Работает со списком всех стен лабиринта. Перемешивает их случайным образом и последовательно убирает стены, если это не создаёт циклов. Использует систему непересекающихся множеств (DSU) для проверки связности. Создаёт лабиринты с равномерным распределением коридоров.",
        ":/resources/gifs/kruskal.gif",
        ":/resources/img/kruskal.png"
    };

    // Прим
    algorithmData["Прим"] = {
        "Алгоритм Прима",
        "Войтех Ярник, 1930 г. / Роберт Прим, 1957 г.",
        "https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%9F%D1%80%D0%B8%D0%BC%D0%B0",
        "Начинает с одной случайной клетки и постепенно 'прорастает', добавляя соседние клетки через случайные стены на границе. Использует список граничных стен. Создаёт лабиринты с короткими тупиками и множеством разветвлений от центра.",
        ":/resources/gifs/prim.gif",
        ":/resources/img/prim.png"
    };

    // BFS (Поиск в ширину)
    algorithmData["BFS (Поиск в Ширину)"] = {
        "BFS (Breadth-First Search) - Поиск в ширину",
        "Эдвард Ф. Мур, 1959 г.",
        "https://ru.wikipedia.org/wiki/%D0%9F%D0%BE%D0%B8%D1%81%D0%BA_%D0%B2_%D1%88%D0%B8%D1%80%D0%B8%D0%BD%D1%83",
        "Исследует лабиринт 'волнами' - сначала все клетки на расстоянии 1, затем на расстоянии 2, и так далее. Использует очередь (FIFO). Гарантированно находит кратчайший путь от старта до финиша. Посещает много клеток, но путь всегда оптимален.",
        ":/resources/gifs/bfs.gif",
        ":/resources/img/bfs.png"
    };

    // A*
    algorithmData["A*"] = {
        "Алгоритм A* (A-star)",
        "П. Харт, Н. Нильссон, Б. Рафаэль, 1968 г.",
        "https://ru.wikipedia.org/wiki/A*",
        "Комбинирует преимущества поиска в ширину и жадного поиска. Использует эвристику (манхэттенское расстояние) для оценки стоимости пути до цели. Идёт целенаправленно к финишу, но гарантирует кратчайший путь. Обычно быстрее BFS, так как посещает меньше клеток.",
        ":/resources/gifs/astar.gif",
        ":/resources/img/astar.png"
    };

    // Дейкстра
    algorithmData["Дейкстра"] = {
        "Алгоритм Дейкстры",
        "Эдсгер В. Дейкстра, 1956 г.",
        "https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%94%D0%B5%D0%B9%D0%BA%D1%81%D1%82%D1%80%D1%8B",
        "Находит кратчайшие пути от начальной вершины до всех остальных. Использует приоритетную очередь для выбора клетки с минимальным расстоянием. В лабиринте без весов работает аналогично BFS, но может учитывать веса рёбер (если они есть).",
        ":/resources/gifs/dejkstra.gif",
        ":/resources/img/dejkstra.png"
    };
}

void InfoWindow::onAlgorithmButtonClicked(const QString &algorithmName)
{
    if (currentAlgorithm == algorithmName) {
        return;
    }

    currentAlgorithm = algorithmName;
    loadAlgorithmInfo(algorithmName);
    updateButtonStyles(algorithmName);
}

QSize InfoWindow::calculateFittedSize(const QSize &origSize, int maxWidth, int maxHeight)
{
    if (origSize.width() <= 0 || origSize.height() <= 0) {
        return QSize(maxWidth, maxHeight);
    }

    double origW = origSize.width();
    double origH = origSize.height();
    double aspect = origW / origH;

    // Пробуем масштабировать по высоте до максимума
    int newH = maxHeight;
    int newW = static_cast<int>(newH * aspect);

    // Если ширина превысила лимит — масштабируем по ширине
    if (newW > maxWidth) {
        newW = maxWidth;
        newH = static_cast<int>(newW / aspect);
    }

    return QSize(newW, newH);
}

void InfoWindow::loadAlgorithmInfo(const QString &algorithmName)
{
    if (!algorithmData.contains(algorithmName)) {
        return;
    }

    const AlgorithmInfo &info = algorithmData[algorithmName];

    // Обновляем текстовые метки
    ui->lab_title->setText(info.title);
    QString refText = QString("<b>%1</b> &nbsp;|&nbsp; <a href='%2'>Подробнее в Википедии</a>")
                          .arg(info.authorYear)
                          .arg(info.referenceUrl);
    ui->lab_ref->setText(refText);
    ui->lab_ref->setOpenExternalLinks(true);
    ui->lab_desc->setText(info.description);
    ui->lab_desc->setWordWrap(true);

    // Ограничения на размер изображений
    // Максимальная ширина — ограничена шириной ScrollArea
    // Максимальная высота — фиксированная (чтобы не растягивать окно)
    int maxImageWidth = 700;
    int maxImageHeight = 600;

    // Загружаем GIF анимацию
    delete currentMovie;
    currentMovie = new QMovie(info.gifPath);

    if (currentMovie->isValid()) {
        // Получаем размер первого кадра
        currentMovie->jumpToFrame(0);
        QSize origSize = currentMovie->currentPixmap().size();

        // Вычисляем итоговый размер с учётом ограничений
        QSize finalSize = calculateFittedSize(origSize, maxImageWidth, maxImageHeight);

        ui->lab_gif->setMovie(currentMovie);
        ui->lab_gif->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->lab_gif->setMinimumWidth(maxImageWidth);
        ui->lab_gif->setFixedHeight(finalSize.height());
        ui->lab_gif->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->lab_gif->setScaledContents(false);  // Не растягиваем содержимое

        currentMovie->jumpToFrame(0);
        currentMovie->start();
    } else {
        ui->lab_gif->setText(QString::fromUtf8("GIF не найден: %1").arg(info.gifPath));
        ui->lab_gif->setFixedSize(400, 300);
    }

    // Загружаем блок-схему
    QPixmap schemaPixmap(info.schemaPath);
    if (!schemaPixmap.isNull()) {
        QSize origSize = schemaPixmap.size();
        QSize finalSize = calculateFittedSize(origSize, maxImageWidth, maxImageHeight);

        // Масштабируем QPixmap до нужного размера
        QPixmap scaledPixmap = schemaPixmap.scaled(
            finalSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );

        ui->lab_sch->setPixmap(scaledPixmap);
        ui->lab_sch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->lab_sch->setMinimumWidth(maxImageWidth);
        ui->lab_sch->setFixedHeight(finalSize.height());
        ui->lab_sch->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    } else {
        ui->lab_sch->setText(QString::fromUtf8("Блок-схема не найдена: %1").arg(info.schemaPath));
        ui->lab_sch->setFixedSize(400, 400);
    }

    // Принудительно обновляем размер внутреннего виджета ScrollArea
    if (ui->scrollArea->widget()) {
        ui->scrollArea->widget()->adjustSize();
    }
}

void InfoWindow::updateButtonStyles(const QString &activeButtonName)
{
    // Стили для кнопок
    QString activeStyle =
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   font-weight: bold;"
        "   border: 2px solid #1976D2;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"
        "}";

    QString inactiveStyle =
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: black;"
        "   border: 1px solid #cccccc;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}";

    // Применяем стили ко всем кнопкам
    ui->but_dfs->setStyleSheet(ui->but_dfs->text() == activeButtonName ? activeStyle : inactiveStyle);
    ui->but_kr->setStyleSheet(ui->but_kr->text() == activeButtonName ? activeStyle : inactiveStyle);
    ui->but_pr->setStyleSheet(ui->but_pr->text() == activeButtonName ? activeStyle : inactiveStyle);
    ui->but_bfs->setStyleSheet(ui->but_bfs->text() == activeButtonName ? activeStyle : inactiveStyle);
    ui->but_ast->setStyleSheet(ui->but_ast->text() == activeButtonName ? activeStyle : inactiveStyle);
    ui->but_dej->setStyleSheet(ui->but_dej->text() == activeButtonName ? activeStyle : inactiveStyle);
}
