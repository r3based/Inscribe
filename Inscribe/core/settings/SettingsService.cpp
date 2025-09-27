#include "SettingsService.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>

static const char *kOrg = "Inscribe";
static const char *kApp = "Inscribe";
static const char *kTectonicPath = "compiler/tectonicPath";
static const char *kWorkspacePath = "projects/workspacePath";
static const char *kCurrentProject = "projects/currentProject";
static const char *kRecentProjects = "projects/recent";
static const char *kAutoBuildOnSave = "build/autoBuildOnSave";

SettingsService::SettingsService()
    : m_settings(new QSettings(kOrg, kApp))
{
}

SettingsService::~SettingsService()
{
    delete m_settings;
}

QString SettingsService::tectonicPath() const
{
    return m_settings->value(kTectonicPath).toString();
}

void SettingsService::setTectonicPath(const QString &path)
{
    m_settings->setValue(kTectonicPath, path);
}

QString SettingsService::workspacePath() const
{
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString def = QDir(home).filePath("Inscribe");
    return m_settings->value(kWorkspacePath, def).toString();
}

void SettingsService::setWorkspacePath(const QString &path)
{
    m_settings->setValue(kWorkspacePath, path);
}

QString SettingsService::currentProjectPath() const
{
    return m_settings->value(kCurrentProject).toString();
}

void SettingsService::setCurrentProjectPath(const QString &path)
{
    m_settings->setValue(kCurrentProject, path);
    addRecentProject(path);
}

QStringList SettingsService::recentProjects() const
{
    return m_settings->value(kRecentProjects).toStringList();
}

void SettingsService::setRecentProjects(const QStringList &paths)
{
    m_settings->setValue(kRecentProjects, paths);
}

void SettingsService::addRecentProject(const QString &path)
{
    QStringList recent = recentProjects();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 10) recent.removeLast();
    setRecentProjects(recent);
}

bool SettingsService::autoBuildOnSave() const
{
    return m_settings->value(kAutoBuildOnSave, false).toBool();
}

void SettingsService::setAutoBuildOnSave(bool on)
{
    m_settings->setValue(kAutoBuildOnSave, on);
}
