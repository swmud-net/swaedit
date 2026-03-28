#ifndef TOOLTIPEVENTFILTER_H
#define TOOLTIPEVENTFILTER_H

#include <QObject>

class ToolTipEventFilter : public QObject {
    Q_OBJECT

public:
    explicit ToolTipEventFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // TOOLTIPEVENTFILTER_H
