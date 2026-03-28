#ifndef PROGRAMSHIGHLIGHTER_H
#define PROGRAMSHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QList>

#include "model/ConfigData.h"

class ProgramsHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    ProgramsHighlighter(QTextDocument *parent,
                        const QList<HighlighterWordsDef> &highlighterDefs);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QList<HighlightingRule> rules_;
};

#endif // PROGRAMSHIGHLIGHTER_H
