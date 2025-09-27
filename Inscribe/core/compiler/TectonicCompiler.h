#ifndef TECTONICCOMPILER_H
#define TECTONICCOMPILER_H

#include "ICompiler.h"

class QProcess;

class TectonicCompiler : public ICompiler
{
    Q_OBJECT
public:
    explicit TectonicCompiler(QObject *parent = nullptr);
    ~TectonicCompiler() override;

    void setProgramPath(const QString &programPath) { m_programPath = programPath; }

    void start(const QString &projectRoot) override;
    void stop() override;

private:
    QProcess *m_process;
    QString m_programPath;
};

#endif // TECTONICCOMPILER_H
