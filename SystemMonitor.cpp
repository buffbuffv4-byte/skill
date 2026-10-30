#include "SystemMonitor.h"
#include <QDir>
#include <QStorageInfo>
#include <QSysInfo>

static ULONGLONG toUll(const FILETIME& ft) {
    ULARGE_INTEGER v;
    v.LowPart = ft.dwLowDateTime; v.HighPart = ft.dwHighDateTime;
    return v.QuadPart;
}

SystemMonitor::SystemMonitor() {}

QString SystemMonitor::detectWindowsVersion() const {
    // QSysInfo avoids relying on deprecated version strings that can be manifest-dependent.
    return QString("%1 (%2-bit)").arg(QSysInfo::prettyProductName())
            .arg(QSysInfo::currentCpuArchitecture().contains("64") ? 64 : 32);
}

MonitorSnapshot SystemMonitor::sample() {
    MonitorSnapshot s;
    FILETIME idleFt{}, kernelFt{}, userFt{};
    if (GetSystemTimes(&idleFt, &kernelFt, &userFt)) {
        ULONGLONG idle = toUll(idleFt), kernel = toUll(kernelFt), user = toUll(userFt);
        if (haveCpuSample) {
            const auto dIdle = idle - oldIdle;
            const auto dKernel = kernel - oldKernel;
            const auto dUser = user - oldUser;
            const auto total = dKernel + dUser;
            if (total > 0) s.cpuPercent = qBound(0.0, 100.0 * (double)(total - qMin(total, dIdle)) / (double)total, 100.0);
        }
        oldIdle = idle; oldKernel = kernel; oldUser = user; haveCpuSample = true;
    }
    MEMORYSTATUSEX mem{}; mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        s.ramTotal = mem.ullTotalPhys; s.ramFree = mem.ullAvailPhys;
        s.ramUsed = s.ramTotal - s.ramFree;
        s.ramPercent = s.ramTotal ? 100.0 * (double)s.ramUsed / (double)s.ramTotal : 0.0;
    }
    QStorageInfo disk(QDir::rootPath());
    if (disk.isValid() && disk.isReady()) {
        s.diskTotal = disk.bytesTotal(); s.diskFree = disk.bytesAvailable();
        s.diskUsed = s.diskTotal >= s.diskFree ? s.diskTotal - s.diskFree : 0;
        s.diskPercent = s.diskTotal ? 100.0 * (double)s.diskUsed / (double)s.diskTotal : -1.0;
    }
    s.uptimeSeconds = GetTickCount64() / 1000;
    s.osName = detectWindowsVersion();
    return s;
}
