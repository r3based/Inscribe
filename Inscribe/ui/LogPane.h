#ifndef LOGPANE_H
#define LOGPANE_H

#include <QWidget>

class QTabWidget;
class QListWidget;
class QPlainTextEdit;

class LogPane : public QWidget
{
    Q_OBJECT
public:
    explicit LogPane(QWidget *parent = nullptr);

public slots:
    void addLine(const QString &text); // backward compat -> appendLog
    void appendLog(const QString &text);
    void appendConsole(const QString &text);
    void clearLogs();

private:
    void scrollToBottom();

    QTabWidget *m_tabs;
    QListWidget *m_logList;
    QPlainTextEdit *m_console;
};

#endif // LOGPANE_H
