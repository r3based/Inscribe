#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QString>
#include <QStringList>

class QSettings;

class SettingsService
{
public:
    SettingsService();
    ~SettingsService();

    QString tectonicPath() const;
    void setTectonicPath(const QString &path);

    QString workspacePath() const;
    void setWorkspacePath(const QString &path);

    QString currentProjectPath() const;
    void setCurrentProjectPath(const QString &path);

    QStringList recentProjects() const;
    void setRecentProjects(const QStringList &paths);
    void addRecentProject(const QString &path);

    bool autoBuildOnSave() const;
    void setAutoBuildOnSave(bool on);

private:
    QSettings *m_settings;
};

#endif // SETTINGSSERVICE_H
