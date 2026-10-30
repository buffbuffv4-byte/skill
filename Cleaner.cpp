#include "Cleaner.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QSet>

bool Cleaner::isSafeTempPath(const QString& path) const {
    const QString temp = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    const QString candidate = QDir::cleanPath(path);
    if (temp.isEmpty() || candidate.size() <= temp.size()) return false;
    const QString prefix = temp.endsWith(QDir::separator()) ? temp : temp + QDir::separator();
    return candidate.startsWith(prefix, Qt::CaseInsensitive);
}
QVector<TempFileInfo> Cleaner::scan(QStringList *warnings) const {
    QVector<TempFileInfo> result;
    const QString root = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (root.isEmpty() || !QDir(root).exists()) {
        if (warnings) warnings->append("Папка временных файлов пользователя не найдена.");
        return result;
    }
    QSet<QString> seen;
    QDirIterator it(root, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = QDir::cleanPath(it.next());
        if (seen.contains(path) || !isSafeTempPath(path)) continue;
        seen.insert(path);
        QFileInfo info(path);
        if (!info.exists() || !info.isFile() || info.isSymLink()) continue;
        result.append({path, (quint64)qMax<qint64>(0, info.size())});
    }
    if (warnings) warnings->append("Проверена только временная папка текущего пользователя. Downloads, Desktop, Documents и системные папки не затрагиваются.");
    return result;
}
CleanResult Cleaner::removeSelected(const QVector<TempFileInfo>& selected) const {
    CleanResult r;
    for (const auto& f : selected) {
        if (!isSafeTempPath(f.path)) { ++r.skipped; r.log << "Пропущен небезопасный путь: " + f.path; continue; }
        QFileInfo info(f.path);
        if (!info.exists() || !info.isFile() || info.isSymLink()) { ++r.skipped; continue; }
        const quint64 size = (quint64)qMax<qint64>(0, info.size());
        QFile file(f.path);
        if (file.remove()) { ++r.deleted; r.bytesDeleted += size; }
        else { ++r.skipped; r.log << "Не удалось удалить (возможно, файл занят или нет доступа): " + f.path; }
    }
    return r;
}
