#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QTimer>
class QVBoxLayout;
#include <QVector>
#include "SystemMonitor.h"
#include "Cleaner.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void updateMonitor();
    void showPage(int index);
    void scanTemp();
    void deleteSelectedTemp();
    void runSystemScan();
private:
    QWidget* makeMetric(const QString& title, QLabel **value, QProgressBar **bar = nullptr);
    QWidget* buildHome();
    QWidget* buildScan();
    QWidget* buildCleaner();
    QWidget* buildMonitor();
    QWidget* buildAbout();
    QWidget* buildUnavailable(const QString& title, const QString& details);
    void addNavButton(const QString& label, int index);
    QString formatBytes(quint64 bytes) const;
    QStackedWidget *pages = nullptr;
    QVBoxLayout *sideLayout = nullptr;
    QLabel *cpuValue=nullptr, *ramValue=nullptr, *ramDetail=nullptr, *diskValue=nullptr, *diskDetail=nullptr, *osValue=nullptr, *uptimeValue=nullptr;
    QProgressBar *cpuBar=nullptr, *ramBar=nullptr, *diskBar=nullptr;
    QLabel *monitorCpu=nullptr, *monitorRam=nullptr, *monitorDisk=nullptr;
    QListWidget *scanResults=nullptr, *tempResults=nullptr;
    QLabel *tempSummary=nullptr;
    QVector<TempFileInfo> tempFiles;
    SystemMonitor monitor;
    Cleaner cleaner;
    QTimer timer;
};
