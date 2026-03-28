#ifndef FLAGSWIDGET_H
#define FLAGSWIDGET_H

#include <QWidget>
#include <QList>
#include <QCheckBox>

#include "model/ConfigData.h"

namespace Ui {
class FlagsWidget;
}

class FlagsWidget : public QWidget {
    Q_OBJECT

public:
    explicit FlagsWidget(const QList<FlagDef> &flagList,
                         qint64 initialValue,
                         const QString &flagsName,
                         bool allowNone = false,
                         QWidget *parent = nullptr);
    ~FlagsWidget();

signals:
    void flagsAccepted(qint64 value);

private slots:
    void onCheckBoxStateChanged(int state);
    void onFlagsValueEditTextChanged(const QString &text);
    void onAcceptClicked();
    void onCancelClicked();

private:
    void updateCheckboxesFromValue();

    Ui::FlagsWidget *ui;
    QList<FlagDef> flagList_;
    qint64 flagsValue_;
    qint64 initialValue_;
    bool updatingFromCode_ = false;
    QList<QCheckBox *> checkboxes_;
};

#endif // FLAGSWIDGET_H
