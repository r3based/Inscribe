#include "LogPane.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTabWidget>

LogPane::LogPane(QWidget *parent)
    : QWidget(parent)
    , m_tabs(new QTabWidget(this))
    , m_logList(new QListWidget(this))
    , m_console(new QPlainTextEdit(this))
{
    m_console->setReadOnly(true);

    m_tabs->addTab(m_logList, QStringLiteral("Logs"));
    m_tabs->addTab(m_console, QStringLiteral("Console"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_tabs);
    setLayout(layout);
}

void LogPane::addLine(const QString &text)
{
    appendLog(text);
}

void LogPane::appendLog(const QString &text)
{
    m_logList->addItem(text);
    scrollToBottom();
    m_tabs->setCurrentIndex(0);
}

void LogPane::appendConsole(const QString &text)
{
    m_console->appendPlainText(text);
    m_console->moveCursor(QTextCursor::End);
    m_tabs->setCurrentIndex(1);
}

void LogPane::clearLogs()
{
    m_logList->clear();
}

void LogPane::scrollToBottom()
{
    if (m_logList->count() > 0)
        m_logList->scrollToItem(m_logList->item(m_logList->count()-1));
}
