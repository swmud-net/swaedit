#ifndef NEWRESETWIDGET_H
#define NEWRESETWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"
#include "model/ConfigData.h"

namespace Ui {
class NewResetWidget;
}

class NewResetWidget : public QWidget {
    Q_OBJECT

public:
    explicit NewResetWidget(Area *area,
                            const QList<ResetInfoDef> &resetsInfo,
                            QWidget *parent = nullptr);
    ~NewResetWidget();

signals:
    void resetCreated();

private slots:
    void onAcceptClicked();
    void onCancelClicked();

private:
    bool requirementMet(const QString &requires) const;

    Ui::NewResetWidget *ui;
    Area *area_;
    QList<ResetInfoDef> resetsInfo_;
};

#endif // NEWRESETWIDGET_H
