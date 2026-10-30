#pragma once
#include <QString>
#include <windows.h>

struct MonitorSnapshot {
    double cpuPercent = -1.0;
    double ramPercent = 0.0;
    quint64 ramUsed = 0;
    quint64 ramTotal = 0;
    quint64 ramFree = 0;
    double diskPercent = -1.0;
    quint64 diskUsed = 0;
    quint64 diskTotal = 0;
    quint64 diskFree = 0;
    quint64 uptimeSeconds = 0;
    QString osName;
};

class SystemMonitor {
public:
    SystemMonitor();
    MonitorSnapshot sample();
private:
    bool haveCpuSample = false;
    ULONGLONG oldIdle = 0, oldKernel = 0, oldUser = 0;
    QString detectWindowsVersion() const;
};
