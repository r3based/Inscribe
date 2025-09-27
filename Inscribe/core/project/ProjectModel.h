#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QString>
#include <QStringList>

class ProjectModel
{
public:
    void setRootPath(const QString &path) { m_root = path; }
    QString rootPath() const { return m_root; }

    static QString defaultWorkspacePath();
    static bool ensureDirectory(const QString &path);

private:
    QString m_root;
};

#endif // PROJECTMODEL_H
