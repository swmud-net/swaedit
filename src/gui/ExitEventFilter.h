#ifndef EXITEVENTFILTER_H
#define EXITEVENTFILTER_H

#include <QObject>

class ExitEventFilter : public QObject {
    Q_OBJECT

public:
    explicit ExitEventFilter(QObject *parent = nullptr);

signals:
    void pressed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // EXITEVENTFILTER_H
