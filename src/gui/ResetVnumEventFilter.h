#ifndef RESETVNUMEVENTFILTER_H
#define RESETVNUMEVENTFILTER_H

#include <QObject>

class ResetVnumEventFilter : public QObject {
    Q_OBJECT

public:
    explicit ResetVnumEventFilter(int argIndex, QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void vnumOverridden(int argIndex, qint64 vnum);

private:
    int argIndex_;
};

#endif // RESETVNUMEVENTFILTER_H
