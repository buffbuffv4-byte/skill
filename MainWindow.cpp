#include "MainWindow.h"
#include "SystemScanner.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QStorageInfo>
#include <QSysInfo>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QCheckBox>
#include <QListWidgetItem>

static QWidget* card(QWidget *child, const QString& heading = {}) {
    auto *frame = new QFrame;
    frame->setObjectName("card");
    auto *lay = new QVBoxLayout(frame);
    lay->setContentsMargins(18,16,18,16);
    lay->setSpacing(10);
    if (!heading.isEmpty()) {
        auto *h = new QLabel(heading); h->setObjectName("cardHeading"); lay->addWidget(h);
    }
    lay->addWidget(child);
    return frame;
}
static QLabel* bodyLabel(const QString& text) {
    auto *l = new QLabel(text); l->setWordWrap(true); l->setObjectName("muted"); return l;
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("SkillOpti 0.1.0 BETA");
    setMinimumSize(940, 620);
    resize(1180, 760);
    auto *central = new QWidget;
    auto *root = new QHBoxLayout(central); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    auto *sidebar = new QFrame; sidebar->setObjectName("sidebar"); sidebar->setFixedWidth(220);
    auto *side = new QVBoxLayout(sidebar); sideLayout = side; side->setContentsMargins(16,20,16,16); side->setSpacing(7);
    auto *brand = new QLabel("SkillOpti"); brand->setObjectName("brand"); side->addWidget(brand);
    auto *ver = new QLabel("0.1.0 BETA"); ver->setObjectName("muted"); side->addWidget(ver);
    side->addSpacing(18);
    addNavButton("⌂   Главная",0); addNavButton("▦   Сканирование ПК",1); addNavButton("♲   Очистка",2);
    addNavButton("◈   Мониторинг",3);
    side->addSpacing(10);
    auto *future = new QLabel("СЛЕДУЮЩИЕ ВЕРСИИ"); future->setObjectName("sectionLabel"); side->addWidget(future);
    addNavButton("⚡   Game Boost",4); addNavButton("▤   Игровые профили",5);
    addNavButton("⇧   Автозагрузка",6); addNavButton("↶   Восстановление",7);
    side->addStretch();
    addNavButton("ⓘ   О программе",8);
    root->addWidget(sidebar);
    pages = new QStackedWidget; pages->setObjectName("pageStack");
    pages->addWidget(buildHome()); pages->addWidget(buildScan()); pages->addWidget(buildCleaner());
    pages->addWidget(buildMonitor());
    pages->addWidget(buildUnavailable("Game Boost","Версия 0.1.0 не завершает процессы. Модуль будет добавлен после отдельной реализации безопасного списка пользовательских процессов."));
    pages->addWidget(buildUnavailable("Игровые профили","Профили CS2, Rust Devblog и Minecraft Java запланированы для следующей версии. В этой версии файлы игр не изменяются."));
    pages->addWidget(buildUnavailable("Автозагрузка","Изменение автозагрузки пока не реализовано. Здесь ничего не отключается и не меняется."));
    pages->addWidget(buildUnavailable("Восстановление","Журнал и откат изменений появятся вместе с функциями, которые будут менять системные настройки. Сейчас изменения системы не выполняются."));
    pages->addWidget(buildAbout());
    root->addWidget(pages,1);
    setCentralWidget(central);
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateMonitor);
    timer.start(1000);
    updateMonitor();
}
void MainWindow::addNavButton(const QString& label, int index) {
    auto *b = new QPushButton(label); b->setObjectName("navButton"); b->setCursor(Qt::PointingHandCursor);
    if (sideLayout) sideLayout->addWidget(b);
    connect(b,&QPushButton::clicked,this,[this,index]{ showPage(index); });
}
void MainWindow::showPage(int index) { pages->setCurrentIndex(index); if(index==1) runSystemScan(); }
QString MainWindow::formatBytes(quint64 b) const {
    const double gb = (double)b/(1024.0*1024*1024);
    if (gb >= 1.0) return QString("%1 ГБ").arg(gb,0,'f',2);
    return QString("%1 МБ").arg((double)b/(1024.0*1024),0,'f',1);
}
QWidget* MainWindow::makeMetric(const QString& title, QLabel **value, QProgressBar **bar) {
    auto *w = new QWidget; auto *l = new QVBoxLayout(w); l->setContentsMargins(0,0,0,0);
    auto *name = new QLabel(title); name->setObjectName("muted");
    *value = new QLabel("Сбор данных…"); (*value)->setObjectName("metricValue");
    l->addWidget(name); l->addWidget(*value);
    if(bar) { *bar = new QProgressBar; (*bar)->setRange(0,100); (*bar)->setValue(0); (*bar)->setTextVisible(false); (*bar)->setFixedHeight(7); l->addWidget(*bar); }
    return w;
}
QWidget* MainWindow::buildHome() {
    auto *page = new QWidget; auto *l = new QVBoxLayout(page); l->setContentsMargins(28,24,28,24); l->setSpacing(18);
    auto *title = new QLabel("Главная"); title->setObjectName("pageTitle"); l->addWidget(title);
    l->addWidget(bodyLabel("Состояние компьютера обновляется автоматически. Значения считываются из Windows API."));
    auto *grid = new QGridLayout; grid->setSpacing(14);
    grid->addWidget(card(makeMetric("Загрузка процессора", &cpuValue, &cpuBar),"CPU"),0,0);
    grid->addWidget(card(makeMetric("Использование памяти", &ramValue, &ramBar),"RAM"),0,1);
    grid->addWidget(card(makeMetric("Использование системного диска", &diskValue, &diskBar),"Диск"),1,0);
    auto *info = new QWidget; auto *il = new QVBoxLayout(info); il->setContentsMargins(0,0,0,0);
    osValue = new QLabel("Сбор данных…"); osValue->setWordWrap(true); osValue->setObjectName("metricValue");
    uptimeValue = new QLabel("Сбор данных…"); uptimeValue->setObjectName("muted");
    il->addWidget(new QLabel("Операционная система")); il->addWidget(osValue); il->addSpacing(10);
    il->addWidget(new QLabel("Время работы")); il->addWidget(uptimeValue);
    grid->addWidget(card(info,"Система"),1,1);
    l->addLayout(grid); l->addStretch(); return page;
}
QWidget* MainWindow::buildScan() {
    auto *page = new QWidget; auto *l = new QVBoxLayout(page); l->setContentsMargins(28,24,28,24); l->setSpacing(14);
    auto *title = new QLabel("Сканирование ПК"); title->setObjectName("pageTitle"); l->addWidget(title);
    l->addWidget(bodyLabel("Характеристики считываются с Windows API и системных интерфейсов. Недоступные данные не подменяются."));
    auto *btn = new QPushButton("Обновить характеристики"); btn->setObjectName("primaryButton"); connect(btn,&QPushButton::clicked,this,&MainWindow::runSystemScan); l->addWidget(btn,0,Qt::AlignLeft);
    scanResults = new QListWidget; scanResults->setObjectName("resultsList"); l->addWidget(scanResults,1);
    runSystemScan(); return page;
}
QWidget* MainWindow::buildCleaner() {
    auto *page = new QWidget; auto *l = new QVBoxLayout(page); l->setContentsMargins(28,24,28,24); l->setSpacing(14);
    auto *title = new QLabel("Очистка временных файлов"); title->setObjectName("pageTitle"); l->addWidget(title);
    l->addWidget(bodyLabel("Сканируется только временная папка текущего пользователя. Личные папки и системные каталоги не затрагиваются. Некоторые файлы могут быть заняты Windows и будут пропущены."));
    auto *row = new QHBoxLayout;
    auto *scan = new QPushButton("Сканировать"); scan->setObjectName("primaryButton"); connect(scan,&QPushButton::clicked,this,&MainWindow::scanTemp);
    auto *del = new QPushButton("Удалить выбранные"); del->setObjectName("dangerButton"); connect(del,&QPushButton::clicked,this,&MainWindow::deleteSelectedTemp);
    row->addWidget(scan); row->addWidget(del); row->addStretch(); l->addLayout(row);
    tempSummary = new QLabel("Нажмите «Сканировать», чтобы найти временные файлы."); tempSummary->setObjectName("muted"); l->addWidget(tempSummary);
    tempResults = new QListWidget; tempResults->setObjectName("resultsList"); tempResults->setSelectionMode(QAbstractItemView::ExtendedSelection); l->addWidget(tempResults,1);
    return page;
}
QWidget* MainWindow::buildMonitor() {
    auto *page = new QWidget; auto *l = new QVBoxLayout(page); l->setContentsMargins(28,24,28,24); l->setSpacing(16);
    auto *title = new QLabel("Мониторинг"); title->setObjectName("pageTitle"); l->addWidget(title);
    l->addWidget(bodyLabel("Текущие показатели обновляются раз в секунду. Исторические графики и датчики температуры требуют отдельной реализации или поддержки оборудования."));
    auto *grid = new QGridLayout; grid->setSpacing(14);
    monitorCpu = new QLabel("Сбор данных…"); monitorRam = new QLabel("Сбор данных…"); monitorDisk = new QLabel("Сбор данных…");
    for (auto *v : {monitorCpu,monitorRam,monitorDisk}) v->setObjectName("metricValue");
    grid->addWidget(card(monitorCpu,"CPU Usage"),0,0); grid->addWidget(card(monitorRam,"RAM Usage"),0,1); grid->addWidget(card(monitorDisk,"Disk Usage"),1,0);
    l->addLayout(grid); l->addStretch(); return page;
}
QWidget* MainWindow::buildAbout() {
    auto *p = new QWidget; auto *l = new QVBoxLayout(p); l->setContentsMargins(28,24,28,24);
    auto *t = new QLabel("О программе"); t->setObjectName("pageTitle"); l->addWidget(t);
    auto *desc = bodyLabel("SkillOpti 0.1.0 BETA\n\nБезопасный мониторинг, диагностика и очистка временных файлов для Windows 10/11.\n\nВ этой версии реализованы мониторинг CPU/RAM/диска, сканирование характеристик и очистка временных файлов с подтверждением. Game Boost, игровые профили, автозагрузка и восстановление пока не выполняют действий.\n\nНе требуется отключать антивирус, менять службы, разгонять оборудование или редактировать реестр.");
    l->addWidget(card(desc,"SkillOpti")); l->addStretch(); return p;
}
QWidget* MainWindow::buildUnavailable(const QString& title, const QString& details) {
    auto *p = new QWidget; auto *l = new QVBoxLayout(p); l->setContentsMargins(28,24,28,24);
    auto *t = new QLabel(title); t->setObjectName("pageTitle"); l->addWidget(t);
    auto *note = bodyLabel(details); l->addWidget(card(note,"В разработке")); l->addStretch(); return p;
}
void MainWindow::updateMonitor() {
    const auto s = monitor.sample();
    if (s.cpuPercent < 0) { if(cpuValue) cpuValue->setText("Сбор данных…"); if(monitorCpu) monitorCpu->setText("Сбор данных…"); }
    else {
        if(cpuValue) cpuValue->setText(QString("%1%").arg(s.cpuPercent,0,'f',0));
        if(cpuBar) cpuBar->setValue((int)s.cpuPercent);
        if(monitorCpu) monitorCpu->setText(QString("%1%").arg(s.cpuPercent,0,'f',1));
    }
    if(ramValue) ramValue->setText(QString("%1%").arg(s.ramPercent,0,'f',0));
    if(ramBar) ramBar->setValue((int)s.ramPercent);
    if(ramDetail) ramDetail->setText(formatBytes(s.ramFree)+" свободно");
    if(monitorRam) monitorRam->setText(QString("%1% · %2 / %3").arg(s.ramPercent,0,'f',1).arg(formatBytes(s.ramUsed),formatBytes(s.ramTotal)));
    if(diskValue) diskValue->setText(s.diskPercent < 0 ? "Недоступно" : QString("%1%").arg(s.diskPercent,0,'f',0));
    if(diskBar && s.diskPercent >= 0) diskBar->setValue((int)s.diskPercent);
    if(diskDetail) diskDetail->setText(formatBytes(s.diskFree)+" свободно");
    if(monitorDisk) monitorDisk->setText(s.diskPercent < 0 ? "Недоступно" : QString("%1% · %2 свободно").arg(s.diskPercent,0,'f',1).arg(formatBytes(s.diskFree)));
    if(osValue) osValue->setText(s.osName);
    if(uptimeValue) {
        quint64 sec=s.uptimeSeconds; const quint64 days=sec/86400; sec%=86400; const quint64 hours=sec/3600; sec%=3600; const quint64 mins=sec/60;
        uptimeValue->setText(QString("%1 дн. %2 ч. %3 мин.").arg(days).arg(hours).arg(mins));
    }
}
void MainWindow::runSystemScan() {
    if(!scanResults) return;
    scanResults->clear();
    for(const auto& line : SystemScanner().scan()) scanResults->addItem(line);
}
void MainWindow::scanTemp() {
    QStringList warnings;
    tempFiles = cleaner.scan(&warnings);
    tempResults->clear();
    quint64 total=0;
    for(const auto& f : tempFiles) {
        total += f.size;
        auto *item = new QListWidgetItem(QString("%1   ·   %2").arg(f.path, formatBytes(f.size)), tempResults);
        item->setData(Qt::UserRole, f.path);
        item->setToolTip(f.path);
    }
    tempSummary->setText(QString("Найдено файлов: %1 · Общий размер: %2\n%3").arg(tempFiles.size()).arg(formatBytes(total)).arg(warnings.join(" ")));
}
void MainWindow::deleteSelectedTemp() {
    const auto selected = tempResults->selectedItems();
    if(selected.isEmpty()) {
        QMessageBox::information(this,"SkillOpti","Сначала выберите один или несколько файлов в списке.");
        return;
    }
    const auto answer = QMessageBox::warning(this,"Подтверждение удаления",
        QString("Удалить выбранные временные файлы (%1 шт.)?\nФайлы будут удалены без помещения в корзину.").arg(selected.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(answer != QMessageBox::Yes) return;
    QVector<TempFileInfo> chosen;
    for(auto *item : selected) {
        const QString path = item->data(Qt::UserRole).toString();
        for(const auto& f : tempFiles) if(f.path == path) { chosen.append(f); break; }
    }
    const auto result = cleaner.removeSelected(chosen);
    QMessageBox::information(this,"Результат очистки",
        QString("Удалено файлов: %1\nПропущено: %2\nОсвобождено: %3")
        .arg(result.deleted).arg(result.skipped).arg(formatBytes(result.bytesDeleted)));
    scanTemp();
}
