#include "TectonicCompiler.h"

#include <QProcess>
#include <QDir>

TectonicCompiler::TectonicCompiler(QObject *parent)
    : ICompiler(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]{
        emit outputReady(QString::fromUtf8(m_process->readAllStandardOutput()));
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this]{
        emit outputReady(QString::fromUtf8(m_process->readAllStandardError()));
    });
    connect(m_process, &QProcess::started, this, [this]{
        emit outputReady(QStringLiteral("[tectonic] process started"));
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err){
        QString msg;
        switch (err) {
        case QProcess::FailedToStart: msg = QStringLiteral("Failed to start (binary not found?)"); break;
        case QProcess::Crashed: msg = QStringLiteral("Crashed"); break;
        case QProcess::Timedout: msg = QStringLiteral("Timed out"); break;
        case QProcess::ReadError: msg = QStringLiteral("Read error"); break;
        case QProcess::WriteError: msg = QStringLiteral("Write error"); break;
        case QProcess::UnknownError: default: msg = QStringLiteral("Unknown error"); break;
        }
        emit outputReady(QStringLiteral("[tectonic] error: ") + msg);
    });
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus){
        CompileResult r; r.exitCode = code;
        emit finished(r);
    });
}

TectonicCompiler::~TectonicCompiler() = default;

void TectonicCompiler::start(const QString &projectRoot)
{
    const QString program = m_programPath.isEmpty() ? QStringLiteral("tectonic") : m_programPath;
    const QString outDir = QStringLiteral("build");

    // Ensure output directory exists
    QDir dir(projectRoot);
    if (!dir.exists(outDir)) {
        if (!dir.mkpath(outDir)) {
            emit outputReady(QStringLiteral("[tectonic] cannot create output directory: ") + dir.filePath(outDir));
        }
    }

    QStringList args;
    args << QStringLiteral("-X") << QStringLiteral("compile")
         << QStringLiteral("main.tex")
         << QStringLiteral("--outdir") << outDir;

    m_process->setProgram(program);
    m_process->setArguments(args);
    m_process->setWorkingDirectory(projectRoot);

    emit outputReady(QStringLiteral("[tectonic] ") + program + " " + args.join(' '));
    m_process->start();
}

void TectonicCompiler::stop()
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();
}
