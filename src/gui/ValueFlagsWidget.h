#ifndef VALUEFLAGSWIDGET_H
#define VALUEFLAGSWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"
#include "model/ConfigData.h"

namespace Ui {
class ValueFlagsWidget;
}

class MainWindow;

class ValueFlagsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ValueFlagsWidget(int valueNo,
                              qint64 flags,
                              const QList<FlagDef> &flagsTree,
                              ObjectValues *values,
                              const QString &title,
                              MainWindow *mainWindow,
                              QWidget *parent = nullptr);
    ~ValueFlagsWidget();

    void setFWFlags(qint64 flags);

signals:
    void textChanged(const QString &text);
    void clicked();

public slots:
    void showFlagsWidget();

private slots:
    void onEditTextChanged(const QString &text);
    void onButtonClicked();

private:
    Ui::ValueFlagsWidget *ui;
    int valueNo_;
    qint64 flags_;
    QList<FlagDef> flagsTree_;
    ObjectValues *values_;
    QString title_;
    MainWindow *mainWindow_;
    bool canEdit_ = true;
};

#endif // VALUEFLAGSWIDGET_H
