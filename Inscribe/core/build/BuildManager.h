#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include <QObject>
#include <QString>

class ICompiler;

class BuildManager : public QObject
{
    Q_OBJECT
public:
    explicit BuildManager(QObject *parent = nullptr);
    ~BuildManager();

    void setCompiler(ICompiler *compiler);

signals:
    void buildStarted();
    void buildOutput(const QString &text);
    void buildFinished(int exitCode);

public slots:
    void startBuild(const QString &projectRoot);
    void stopBuild();

private:
    ICompiler *m_compiler;
    QString m_projectRoot;
};

#endif // BUILDMANAGER_H
