#ifndef ICOMPILER_H
#define ICOMPILER_H

#include <QObject>

struct CompileResult {
    int exitCode = -1;
    QString stdOut;
    QString stdErr;
};

class ICompiler : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~ICompiler() = default;

    virtual void start(const QString &projectRoot) = 0;
    virtual void stop() = 0;

signals:
    void outputReady(const QString &text);
    void finished(const CompileResult &result);
};

#endif // ICOMPILER_H
