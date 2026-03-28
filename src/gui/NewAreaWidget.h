#ifndef NEWAREAWIDGET_H
#define NEWAREAWIDGET_H

#include <QWidget>

#include "model/Area.h"

namespace Ui {
class NewAreaWidget;
}

class NewAreaWidget : public QWidget {
    Q_OBJECT

public:
    explicit NewAreaWidget(QWidget *parent = nullptr);
    ~NewAreaWidget();

signals:
    void areaCreated(Area area);

private slots:
    void onAcceptClicked();
    void onCancelClicked();
    void onOtherRadioToggled(bool checked);

private:
    Ui::NewAreaWidget *ui;
};

#endif // NEWAREAWIDGET_H
