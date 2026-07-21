#include "infowindow.h"
#include "ui_infowindow.h"
#include <QPixmap>
#include <QButtonGroup>

InfoWindow::InfoWindow(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow)
    , currentMovie(nullptr)
{
    ui->setupUi(this);
    setupAlgorithmData();

    // Группируем кнопки, чтобы они работали как переключатели
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->but_dfs, 0);
    buttonGroup->addButton(ui->but_kr, 1);
    buttonGroup->addButton(ui->but_pr, 2);
    buttonGroup->addButton(ui->but_ast, 3);
    buttonGroup->addButton(ui->but_dej, 4);
    buttonGroup->addButton(ui->but_bfs, 5);

    connect(buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
        this, [this](QAbstractButton* button) {
            onAlgorithmButtonClicked(button->text());
        });

    // По умолчанию показываем DFS
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
        "https://research.ibm.com/publications?search=eyJ0eXBlIjoic2VhcmNoIiwidmFsdWUiOiJERlMifQ",
        "Алгоритм начинает с произвольной клетки и идёт вглубь лабиринта, пока не упрётся в тупик. Затем отступает назад (backtracking) и пробует другое направление. Использует стек для отслеживания пути. Создаёт лабиринты с длинными извилистыми коридорами.",
        QString::fromUtf8(
            "• Обход графов и деревьев в информатике\n"
            "• Топологическая сортировка зависимостей (системы сборки, планирование задач)\n"
            "• Решение головоломок: судоку, кроссворды, лабиринты\n"
            "• Поиск компонент сильной связности в ориентированных графах (алгоритм Тарьяна)\n"
            "• Генерация случайных лабиринтов и процедурная генерация в играх\n"
            "• Анализ выражений в компиляторах (обход синтаксических деревьев)"),
        ":/resources/gifs/dfs.gif",
        ":/resources/img/dfs.png"
    };

    // Крускал
    algorithmData["Краскал"] = {
        "Алгоритм Краскала",
        "Джозеф Крускал, 1956 г.",
        "https://research.ibm.com/publications?search=eyJ0eXBlIjoic2VhcmNoIiwidmFsdWUiOiJLcnVza2FsIn0",
        "Работает со списком всех стен лабиринта. Перемешивает их случайным образом и последовательно убирает стены, если это не создаёт циклов. Использует систему непересекающихся множеств (DSU) для проверки связности. Создаёт лабиринты с равномерным распределением коридоров.",
        QString::fromUtf8(
            "• Проектирование минимальных сетей: дороги, трубопроводы, электросети\n"
            "• Кластеризация данных в машинном обучении (однопроходная кластеризация)\n"
            "• Построение приближённых решений задачи коммивояжёра\n"
            "• Сети связи: минимизация стоимости прокладки кабелей между узлами\n"
            "• Распознавание образов и сегментация изображений\n"
            "• Биоинформатика: построение филогенетических деревьев"),
        ":/resources/gifs/kruskal.gif",
        ":/resources/img/kruskal.png"
    };

    // Прим
    algorithmData["Прим"] = {
        "Алгоритм Прима",
        "Войтех Ярник, 1930 г. / Роберт Прим, 1957 г.",
        "https://research.ibm.com/publications?search=eyJ0eXBlIjoic2VhcmNoIiwidmFsdWUiOiJQcmltIn0",
        "Начинает с одной случайной клетки и постепенно 'прорастает', добавляя соседние клетки через случайные стены на границе. Использует список граничных стен. Создаёт лабиринты с короткими тупиками и множеством разветвлений от центра.",
        QString::fromUtf8(
            "• Проектирование сетей связи от центрального узла (хаба)\n"
            "• Размещение сетевой инфраструктуры в зданиях и городах\n"
            "• Задачи, где важно 'расти' от уже построенной части сети\n"
            "• Плотные графы (эффективнее алгоритма Крускала на таких данных)\n"
            "• Сети вещания (broadcast networks) с единым источником\n"
            "• Оптимизация транспортных маршрутов от склада"),
        ":/resources/gifs/prim.gif",
        ":/resources/img/prim.png"
    };

    // BFS (Поиск в Ширину)
    algorithmData["BFS (Поиск в Ширину)"] = {
        "BFS (Breadth-First Search) - Поиск в ширину",
        "Эдвард Ф. Мур, 1959 г.",
        "https://research.ibm.com/publications?search=eyJ0eXBlIjoic2VhcmNoIiwidmFsdWUiOiJCRlMifQ",
        "Исследует лабиринт 'волнами' - сначала все клетки на расстоянии 1, затем на расстоянии 2, и так далее. Использует очередь (FIFO). Гарантированно находит кратчайший путь от старта до финиша. Посещает много клеток, но путь всегда оптимален.",
        QString::fromUtf8(
            "• Поиск кратчайшего пути в невзвешенных графах\n"
            "• Веб-краулеры поисковых систем (Google, Yandex) для обхода страниц\n"
            "• Социальные сети: поиск связей между пользователями (задача '6 рукопожатий')\n"
            "• Сетевые протоколы: широковещательная рассылка пакетов\n"
            "• Проверка графа на двудольность\n"
            "• GPS-навигация в метро и городских сетях с равномерными участками\n"
            "• Искусственный интеллект: поиск в играх с равной стоимостью ходов"),
        ":/resources/gifs/bfs.gif",
        ":/resources/img/bfs.png"
    };

    // A*
    algorithmData["A*"] = {
        "Алгоритм A* (A-star)",
        "П. Харт, Н. Нильссон, Б. Рафаэль, 1968 г.",
        "https://research.ibm.com/publications/evaluation-of-a-simple-scalable-parallel-best-first-search-strategy",
        "Комбинирует преимущества поиска в ширину и жадного поиска. Использует эвристику (манхэттенское расстояние) для оценки стоимости пути до цели. Идёт целенаправленно к финишу, но гарантирует кратчайший путь. Обычно быстрее BFS, так как посещает меньше клеток.",
        QString::fromUtf8(
            "• Игровая индустрия: поиск пути для NPC (Unity, Unreal Engine)\n"
            "• Карты и навигация: Google Maps, Яндекс.Карты, 2ГИС\n"
            "• Робототехника: планирование траекторий мобильных роботов\n"
            "• Логистика: оптимизация маршрутов доставки\n"
            "• Биоинформатика: выравнивание последовательностей ДНК\n"
            "• Обработка естественного языка: поиск в грамматических графах\n"
            "• Автоматическое планирование в системах ИИ"),
        ":/resources/gifs/astar.gif",
        ":/resources/img/astar.png"
    };

    // Дейкстра
    algorithmData["Дейкстра"] = {
        "Алгоритм Дейкстры",
        "Эдсгер В. Дейкстра, 1956 г.",
        "https://research.ibm.com/publications?search=eyJ0eXBlIjoic2VhcmNoIiwidmFsdWUiOiJEaWprc3RyYSJ9",
        "Находит кратчайшие пути от начальной вершины до всех остальных. Использует приоритетную очередь для выбора клетки с минимальным расстоянием. В лабиринте без весов работает аналогично BFS, но может учитывать веса рёбер (если они есть).",
        QString::fromUtf8(
            "• Маршрутизация в компьютерных сетях: протоколы OSPF и IS-IS\n"
            "• GPS-навигация: расчёт маршрутов с учётом пробок и расстояний\n"
            "• Социальные сети: рекомендации связей между пользователями\n"
            "• Телекоммуникации: оптимизация потоков данных\n"
            "• Авиаперелёты: поиск cheapest routes между аэропортами\n"
            "• Финансовые технологии: арбитражные пути на биржах\n"
            "• Базы данных: алгоритм D* в СУБД для оптимизации запросов"),
        ":/resources/gifs/dejkstra.gif",
        ":/resources/img/dejkstra.png"
    };
}

void InfoWindow::onAlgorithmButtonClicked(const QString& algorithmName)
{
    // Не перезагружаем тот же алгоритм повторно
    if (currentAlgorithm == algorithmName) {
        return;
    }
    currentAlgorithm = algorithmName;
    loadAlgorithmInfo(algorithmName);
    updateButtonStyles(algorithmName);
}

QSize InfoWindow::calculateFittedSize(const QSize& origSize, int maxWidth, int maxHeight)
{
    if (origSize.width() <= 0 || origSize.height() <= 0) {
        return QSize(maxWidth, maxHeight);
    }

    double origW = origSize.width();
    double origH = origSize.height();
    double aspect = origW / origH;

    // Сначала пробуем вписать по высоте
    int newH = maxHeight;
    int newW = static_cast<int>(newH * aspect);

    // Если ширина вылезла — масштабируем по ширине
    if (newW > maxWidth) {
        newW = maxWidth;
        newH = static_cast<int>(newW / aspect);
    }

    return QSize(newW, newH);
}

void InfoWindow::loadAlgorithmInfo(const QString& algorithmName)
{
    if (!algorithmData.contains(algorithmName)) {
        return;
    }

    const AlgorithmInfo& info = algorithmData[algorithmName];

    // Заголовок
    ui->lab_title->setText(info.title);

    // Ссылка: автор + кликабельный текст
    QString refText = QString("<b>%1</b> &nbsp;|&nbsp; <a href='%2'>Подробнее в научных статьях на сайте IBM Research</a>")
        .arg(info.authorYear)
        .arg(info.referenceUrl);
    ui->lab_ref->setText(refText);
    ui->lab_ref->setOpenExternalLinks(true);

    // Описание и область применения
    ui->lab_desc->setText(info.description);
    ui->lab_desc->setWordWrap(true);

    ui->lab_use->setText(QString(info.application));
    ui->lab_use->setWordWrap(true);

    // Ограничения на размер картинок
    int maxImageWidth = 700;
    int maxImageHeight = 600;

    // GIF-анимация
    delete currentMovie;
    currentMovie = new QMovie(info.gifPath);

    if (currentMovie->isValid()) {
        currentMovie->jumpToFrame(0);
        QSize origSize = currentMovie->currentPixmap().size();
        QSize finalSize = calculateFittedSize(origSize, maxImageWidth, maxImageHeight);

        ui->lab_gif->setMovie(currentMovie);
        ui->lab_gif->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->lab_gif->setMinimumWidth(maxImageWidth);
        ui->lab_gif->setFixedHeight(finalSize.height());
        ui->lab_gif->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->lab_gif->setScaledContents(false);

        currentMovie->jumpToFrame(0);
        currentMovie->start();
    }
    else {
        ui->lab_gif->setText(QString::fromUtf8("GIF не найден: %1").arg(info.gifPath));
        ui->lab_gif->setFixedSize(400, 300);
    }

    // Блок-схема
    QPixmap schemaPixmap(info.schemaPath);
    if (!schemaPixmap.isNull()) {
        QSize origSize = schemaPixmap.size();
        QSize finalSize = calculateFittedSize(origSize, maxImageWidth, maxImageHeight);

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
    }
    else {
        ui->lab_sch->setText(QString::fromUtf8("Блок-схема не найдена: %1").arg(info.schemaPath));
        ui->lab_sch->setFixedSize(400, 400);
    }

    // Пересчитываем размер внутреннего виджета ScrollArea
    if (ui->scrollArea->widget()) {
        ui->scrollArea->widget()->adjustSize();
    }
}

void InfoWindow::updateButtonStyles(const QString& activeButtonName)
{
    // Стили для активной и неактивной кнопок
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

    // Прогоняем все кнопки через один цикл — не дублируем код
    QList<QPushButton*> buttons = { ui->but_dfs, ui->but_kr, ui->but_pr,
                                   ui->but_bfs, ui->but_ast, ui->but_dej };
    for (QPushButton* btn : buttons) {
        btn->setStyleSheet(btn->text() == activeButtonName ? activeStyle : inactiveStyle);
    }
}