#include "Cleaner.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QSet>

namespace {
// Каталог временных файлов текущего пользователя в «чистом» виде (разделитель '/').
QString tempRoot() {
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
}
// Каноническая форма (раскрывает 8.3-имена, ссылки, junction).
QString canonicalTempRoot() {
    const QString root = tempRoot();
    return root.isEmpty() ? QString() : QFileInfo(root).canonicalFilePath();
}
bool isStrictlyInside(const QString& rootDir, const QString& path) {
    if (rootDir.isEmpty() || path.size() <= rootDir.size()) return false;
    // QDir::cleanPath() всегда возвращает '/', поэтому QDir::separator() ('\\' на Windows) здесь использовать нельзя.
    const QString prefix = rootDir.endsWith(QLatin1Char('/')) ? rootDir : rootDir + QLatin1Char('/');
    return path.startsWith(prefix, Qt::CaseInsensitive);
}
} // namespace

bool Cleaner::isSafeTempPath(const QString& path) const {
    const QString temp = tempRoot();
    const QString candidate = QDir::cleanPath(path);
    if (!isStrictlyInside(temp, candidate)) return false;

    // Дополнительная защита: настоящее расположение файла (после раскрытия ссылок/junction)
    // тоже должно находиться внутри настоящей временной папки.
    const QString canonRoot = canonicalTempRoot();
    const QString canonFile = QFileInfo(candidate).canonicalFilePath();
    if (canonRoot.isEmpty() || canonFile.isEmpty()) return false;
    return isStrictlyInside(QDir::cleanPath(canonRoot), QDir::cleanPath(canonFile));
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
        TempFileInfo entry;
        entry.path = path;
        entry.size = static_cast<quint64>(qMax<qint64>(0, info.size()));
        result.append(entry);
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
        const quint64 size = static_cast<quint64>(qMax<qint64>(0, info.size()));
        QFile file(f.path);
        if (file.remove()) { ++r.deleted; r.bytesDeleted += size; }
        else { ++r.skipped; r.log << "Не удалось удалить (возможно, файл занят или нет доступа): " + f.path; }
    }
    return r;
}
