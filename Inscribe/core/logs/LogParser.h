#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QString>
#include <QVector>

struct Diagnostic {
    enum class Severity { Info, Warning, Error };
    Severity severity = Severity::Info;
    QString filePath;
    int line = -1;
    QString message;
};

class LogParser
{
public:
    QVector<Diagnostic> parse(const QString &rawLog) const;
};

#endif // LOGPARSER_H
