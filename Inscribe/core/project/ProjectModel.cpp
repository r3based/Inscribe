#include "ProjectModel.h"

#include <QStandardPaths>
#include <QDir>

QString ProjectModel::defaultWorkspacePath()
{
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return QDir(home).filePath("Inscribe");
}

bool ProjectModel::ensureDirectory(const QString &path)
{
    QDir dir(path);
    if (dir.exists()) return true;
    return dir.mkpath(".");
}
