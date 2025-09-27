#include "BuildManager.h"

#include <QObject>

#include "core/compiler/ICompiler.h"

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
    , m_compiler(nullptr)
{
}

BuildManager::~BuildManager() = default;

void BuildManager::setCompiler(ICompiler *compiler)
{
    if (m_compiler == compiler) return;
    if (m_compiler) {
        m_compiler->disconnect(this);
    }
    m_compiler = compiler;
    if (m_compiler) {
        connect(m_compiler, &ICompiler::outputReady, this, &BuildManager::buildOutput);
        connect(m_compiler, &ICompiler::finished, this, [this](const CompileResult &r){
            emit buildFinished(r.exitCode);
        });
    }
}

void BuildManager::startBuild(const QString &projectRoot)
{
    m_projectRoot = projectRoot;
    emit buildStarted();
    if (m_compiler) {
        m_compiler->start(projectRoot);
    } else {
        emit buildOutput(QStringLiteral("No compiler configured"));
        emit buildFinished(-1);
    }
}

void BuildManager::stopBuild()
{
    if (m_compiler) m_compiler->stop();
}
