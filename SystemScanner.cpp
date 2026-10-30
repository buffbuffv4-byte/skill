#include "SystemScanner.h"
#include <QSysInfo>
#include <QStorageInfo>
#include <QDir>
#include <QRegularExpression>
#include <cstring>
#include <windows.h>
#include <intrin.h>

static QString cpuName() {
    int regs[4] = {};
    __cpuid(regs, 0x80000000);
    unsigned maxExt = (unsigned)regs[0];
    if (maxExt < 0x80000004) return "Не удалось определить";
    char brand[49] = {};
    for (unsigned i = 0; i < 3; ++i) {
        __cpuid(regs, 0x80000002 + i);
        memcpy(brand + i * 16, regs, 16);
    }
    return QString::fromLocal8Bit(brand).trimmed();
}
static QString gpuName() {
    // Query display adapter names through the Windows display-device API.
    DISPLAY_DEVICEW dd{}; dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && dd.DeviceString[0])
            return QString::fromWCharArray(dd.DeviceString);
        ZeroMemory(&dd, sizeof(dd)); dd.cb = sizeof(dd);
    }
    return "Не удалось определить";
}
QStringList SystemScanner::scan() const {
    QStringList out;
    SYSTEM_INFO si{}; GetNativeSystemInfo(&si);
    out << "Процессор: " + cpuName()
        << "Видеокарта / адаптер: " + gpuName()
        << QString("Логических потоков CPU: %1").arg(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS))
        << QString("Архитектура системы: %1").arg(QSysInfo::currentCpuArchitecture())
        << "Операционная система: " + QSysInfo::prettyProductName();
    MEMORYSTATUSEX mem{}; mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem))
        out << QString("Оперативная память: %1 ГБ").arg((double)mem.ullTotalPhys / (1024.0*1024*1024), 0, 'f', 2);
    else out << "Оперативная память: Не удалось определить";
    out << "Накопители:";
    const auto drives = QStorageInfo::mountedVolumes();
    bool found = false;
    for (const auto& d : drives) {
        if (!d.isValid() || !d.isReady() || d.bytesTotal() <= 0) continue;
        found = true;
        out << QString("  %1 — %2 ГБ всего, %3 ГБ свободно")
               .arg(d.rootPath())
               .arg((double)d.bytesTotal()/(1024.0*1024*1024), 0, 'f', 1)
               .arg((double)d.bytesAvailable()/(1024.0*1024*1024), 0, 'f', 1);
    }
    if (!found) out << "  Не удалось определить";
    return out;
}
