#ifndef RESETFLAGSWIDGET_H
#define RESETFLAGSWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"
#include "model/ConfigData.h"

namespace Ui {
class ValueFlagsWidget;
}

class MainWindow;

class ResetFlagsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResetFlagsWidget(int argIndex,
                              qint64 flags,
                              const QList<FlagDef> &flagsTree,
                              AreaReset *reset,
                              const QString &title,
                              MainWindow *mainWindow,
                              QWidget *parent = nullptr);
    ~ResetFlagsWidget();

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
    int argIndex_;
    qint64 flags_;
    QList<FlagDef> flagsTree_;
    AreaReset *reset_;
    QString title_;
    MainWindow *mainWindow_;
    bool canEdit_ = true;
};

#endif // RESETFLAGSWIDGET_H
