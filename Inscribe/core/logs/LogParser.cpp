#include "LogParser.h"

QVector<Diagnostic> LogParser::parse(const QString &rawLog) const
{
    QVector<Diagnostic> out;
    const QStringList lines = rawLog.split('\n');
    out.reserve(lines.size());
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;
        Diagnostic d;
        d.severity = Diagnostic::Severity::Info;
        d.message = line;
        out.push_back(d);
    }
    return out;
}
