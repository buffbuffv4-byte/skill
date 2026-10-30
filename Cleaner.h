#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

struct TempFileInfo {
    QString path;
    quint64 size = 0;
};
struct CleanResult {
    int deleted = 0;
    int skipped = 0;
    quint64 bytesDeleted = 0;
    QStringList log;
};
class Cleaner {
public:
    QVector<TempFileInfo> scan(QStringList *warnings = nullptr) const;
    CleanResult removeSelected(const QVector<TempFileInfo>& selected) const;
private:
    bool isSafeTempPath(const QString& path) const;
};
