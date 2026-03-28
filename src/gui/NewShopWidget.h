#ifndef NEWSHOPWIDGET_H
#define NEWSHOPWIDGET_H

#include <QWidget>

#include "model/Area.h"

namespace Ui {
class NewShopWidget;
}

class NewShopWidget : public QWidget {
    Q_OBJECT

public:
    explicit NewShopWidget(Area *area, bool isRepair, QWidget *parent = nullptr);
    ~NewShopWidget();

signals:
    void vnumChosen(int vnum);

private slots:
    void onAcceptClicked();
    void onCancelClicked();

private:
    Ui::NewShopWidget *ui;
    Area *area_;
    bool isRepair_;
};

#endif // NEWSHOPWIDGET_H
