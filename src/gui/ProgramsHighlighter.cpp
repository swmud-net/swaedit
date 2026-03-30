#include "gui/ProgramsHighlighter.h"

#include <QColor>

ProgramsHighlighter::ProgramsHighlighter(QTextDocument *parent,
                                         const QList<HighlighterWordsDef> &highlighterDefs)
    : QSyntaxHighlighter(parent)
{
    for (const HighlighterWordsDef &def : highlighterDefs) {
        // Parse color from hex string like "0x8B008B"
        QColor color;
        if (def.color != "0" && !def.color.isEmpty()) {
            QString colorStr = def.color;
            if (colorStr.startsWith("0x") || colorStr.startsWith("0X")) {
                colorStr = colorStr.mid(2);
            }
            bool ok;
            unsigned int rgb = colorStr.toUInt(&ok, 16);
            if (ok) {
                color = QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
            } else {
                color = Qt::black;
            }
        } else {
            color = Qt::black;
        }

        QTextCharFormat format;
        format.setForeground(color);

        bool isVariable = (def.type == "variable" || def.type == "ratmVariable");

        for (const QString &word : def.words) {
            HighlightingRule rule;
            if (isVariable) {
                rule.pattern = QRegularExpression("\\$\\b" + QRegularExpression::escape(word) + "\\b");
            } else {
                rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(word) + "\\b");
            }
            rule.format = format;
            rules_.append(rule);
        }
    }
}

void ProgramsHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : rules_) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
