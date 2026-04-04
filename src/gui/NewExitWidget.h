#ifndef NEWEXITWIDGET_H
#define NEWEXITWIDGET_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QToolButton>

#include "model/Area.h"
#include "model/ConfigData.h"

namespace Ui {
class NewExitWidget;
}

class NewExitWidget : public QWidget {
    Q_OBJECT

public:
    explicit NewExitWidget(Room *room,
                           const QList<ExitDef> &exits,
                           int gridColumns,
                           Area *area,
                           const QMap<int, ExitDef> &exitsMap,
                           QWidget *parent = nullptr);
    ~NewExitWidget();

signals:
    void exitCreated();

private slots:
    void onDirectionButtonClicked();
    void onOneWayToggled(bool checked);
    void onTwoWayToggled(bool checked);
    void onAcceptClicked();
    void onCancelClicked();

private:
    void fillDestinationBox();
    bool roomHasExitInDirection(const Room *room, int direction) const;

    Ui::NewExitWidget *ui;
    Room *room_;
    QList<ExitDef> exits_;
    Area *area_;
    QMap<int, ExitDef> exitsMap_;
    int selectedDirection_ = -1;
    QToolButton *selectedButton_ = nullptr;
    QMap<QToolButton *, int> buttonDirectionMap_;
};

#endif // NEWEXITWIDGET_H
